#include "taxiorder.h"

ITaxiOrder::ITaxiOrder(int _order_id, TaxiRatePeriod _taxiRate, float _parkingCost, int _parkingId, QObject *parent)
	: QObject(parent), 
	
	_order_id(_order_id), 
	
	_mileage_city(0), 
	
	_total_travel_time_seconds(0), 
	seconds_stops(0), seconds_moving(0), seconds_client_stops(0), seconds_traincross_stops(0),
	
	_out_of_city_rate(0),
	
	_destination_region_id(0),
	taxiRate(_taxiRate),
	_is_talon(false),
	
	gotPosition(false), 
	
	started(false), movementStarted(false), _totalTimeStarted(false),
	
	parkingCost(_parkingCost), parkingId(_parkingId),
	current_stop_seconds(0), 
	
	outOfCity(false),

	_clientStop(false),_overload(false),_trainCross(false),
	
	_mileage_out_of_city(0), _mileage_city_overload(0), _mileage_out_of_city_overload(0)
{
	qDebug() << "newOrder id:" << _order_id << "rate:" << taxiRate.car_in() << " " << taxiRate.km_g();
	paymentTimer = new QTimer(this);
	
	connect(paymentTimer, SIGNAL(timeout()), SLOT(measureTimes()));
	connect(paymentTimer, SIGNAL(timeout()), SLOT(recalcSum()));
	paymentTimer->setInterval(1000);
	paymentTimer->start();
}

ITaxiOrder::~ITaxiOrder()
{

}

void ITaxiOrder::measureTimes()
{
	// если мы на переезде и т.д. нам всё равно нужно время считать,
	// но только после того, как вообще началась поездка
	if (_totalTimeStarted) {
		_total_travel_time_seconds++;
		emit newTimeTotal(_total_travel_time_seconds);
	}

	// заказ либо на паузе, либо не начался
	if (!started)
		return;

	
	if (movementStarted) {
		seconds_moving++;
	}

	// новые совмещённые остановки
	if (_clientStop && !movementStarted)	
	{
		// тут была фильтрация по времени
		emit movementStartFiltered(false);
		seconds_client_stops++;
	}

	emit newTimeMovement(seconds_moving);
	emit newTimeStops(seconds_stops);
	emit newTimeClientStops(seconds_client_stops);
}

void ITaxiOrder::setRegionId(int _region_id)
{
	_destination_region_id = _region_id;
	emit regionChanged(_destination_region_id);
}

int ITaxiOrder::getRegionId()
{
	return _destination_region_id;
}

double ITaxiOrder::totalMileage()
{
	return cityMileage() + outOfCityMileage();
}

#define ROUND_UPPER(val) (((int)((val + 0.5) * 10.0)) / 10)

double ITaxiOrder::moneyCity()
{	
	double mileage_city_cost = cityMileage() * taxiRate.km_g();	

	return mileage_city_cost + moneyCityOverload();
}

double ITaxiOrder::moneyCityOverload() 
{
	double mileage_city_overload_cost = cityMileageOverload() * taxiRate.km_g() * 1.5;

	return mileage_city_overload_cost;
}

int ITaxiOrder::moneyMg()
{
	double mileage_out_of_city_cost = outOfCityMileage() * mgRate();

	return ROUND_UPPER(mileage_out_of_city_cost + 
		moneyMgOverload());
}

double ITaxiOrder::moneyMgOverload()
{
	double mileage_out_of_city_overload_cost = outOfCityMileageOverload() * mgRate() * 1.5;

	return mileage_out_of_city_overload_cost;
}

/* ============================================================= */
int ITaxiOrder::calculateSum()
{
	// подача машины -- с платной стоянки
	double car_in = getCarIn();
		
	// остановки по просьбе клиента
	double client_stops = minutesClientStops() * taxiRate.client_stop();

	// пробки
	// double stops = taxiRate.car_min() * 0.5 * minutesStops();

	int value = ROUND_UPPER(car_in + moneyCity() + client_stops) +
				   moneyMg();
	
	// округляем рубли к ближайшему
	return value;
}
/* ============================================================= */

void ITaxiOrder::recalcSum()
{
	float payment = calculateSum();
	emit paymentChanged(payment);
}

void ITaxiOrder::movementStart(int startStop)
{
	if (startStop) {
		// начали ехать
		movementStarted = true;
	} else {
		// остановка
		movementStarted = false;
	}
}

void ITaxiOrder::setTrainCross(bool on)
{
	_trainCross = on;
	if (on) {
		stopOrder();
	} else {
		startOrder();
	}
}

void ITaxiOrder::newPosition(QGeoCoordinate newPosition)
{
	if (!started)
		return;
	
	if (gotPosition) {
		if (outOfCity) {
			if (_overload) {
				_mileage_out_of_city_overload += newPosition.distanceTo(currentPosition);
			} else {
				_mileage_out_of_city += newPosition.distanceTo(currentPosition);
			}
		} else {
			if (_overload) {
				_mileage_city_overload += newPosition.distanceTo(currentPosition);
			} else {
				_mileage_city += newPosition.distanceTo(currentPosition);
			}
		}
		//emit newMileage((_mileage_out_of_city + _mileage_city + _mileage_city_overload + _mileage_out_of_city_overload) / 1000.0);
		// все округлены по 100 метров и сложены
		emit newMileage(cityMileage() + outOfCityMileage() + cityMileageOverload() + outOfCityMileageOverload());
		recalcSum();
	}
	
	currentPosition = newPosition;
	gotPosition = true;

}

void ITaxiOrder::startOrder()
{
	started = true;
}

void ITaxiOrder::stopOrder()
{
	started = false;
}

int ITaxiOrder::getOrderId()
{
	return _order_id;
}

void ITaxiOrder::setOrderId(int order_id)
{
	_order_id = order_id;
}