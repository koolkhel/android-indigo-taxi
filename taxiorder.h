#ifndef TAXIORDER_H
#define TAXIORDER_H

#include <QtCore>

#include "hello.pb.h"
#include "qgeocoordinate.h"

#define NO_ORDER_ID -1

class ITaxiOrder : public QObject
{
	Q_OBJECT

public:
	ITaxiOrder(int _order_id, TaxiRatePeriod _taxiRate, float _parkingCost, int _parkingId, QObject *parent);
	~ITaxiOrder();

	enum {
		NO_STATUS = 0,
		SUCCESS = 1,
		ABORT_TIMEOUT = 2,
		ABORT_DISPATCHER = 3,
		NOT_PAY = 4,
		NOT_EXIT = 5,
		NOT_TO_ME = 6,
		EMPTY_TRIP = 7
	};

	void setRegionId(int _region_id);
	int getRegionId();
	int getOrderId();
	void setOrderId(int order_id);
	int calculateSum();
	TaxiRatePeriod orderTaxiRate() { return taxiRate; }
	void setIsTalon(bool talon) { _is_talon = talon; }
	bool getIsTalon() { return _is_talon; }
    void setBonus(bool bonus) { _is_bonus = bonus; }
    bool getBonus() { return _is_bonus; }
    void setBonusSeconds(int seconds) { _bonus_seconds = seconds; }
    int getBonusSeconds() { return _bonus_seconds; }
    bool isBonusTime() {
        return _is_bonus && (_total_travel_time_seconds < _bonus_seconds);
    }

	void setOrderTaxiRate(TaxiRatePeriod _rate) { taxiRate = _rate; } 
	double totalMileage();
	
	// километражи округляются до сотен метров
	float cityMileage() { return (((int)_mileage_city + 50) / 100) / 10.0; }
	float outOfCityMileage() { return (((int)_mileage_out_of_city + 50) / 100) / 10.0; }
	float cityMileageOverload() { return (((int)_mileage_city_overload + 50) / 100) / 10.0; }
	float outOfCityMileageOverload() { return (((int)_mileage_out_of_city_overload + 50) / 100) / 10.0; }

	//double car_in = taxiRate.car_in() + parkingCost;	
	int getCarIn() { return taxiRate.car_in(); }

	double moneyCity();
	double moneyCityOverload();

	int moneyMg();
	double moneyMgOverload();
	
	void setOverload(bool overload) { _overload = overload; }
	void setClientStop(bool clientStop) { _clientStop = clientStop; }
	void setTrainCross(bool on);
	
	
	float mgRate() { return _out_of_city_rate;}
	int getParkingId() { return parkingId; }
	float getParkingCost() { return parkingCost; }
	
	// время округляется до минут
	int minutesTraincrossStops() {return (seconds_traincross_stops + 30) / 60;}
	int secondsTraincrossStops() { return seconds_traincross_stops; }
    int minutesClientStops() {
        // требование бригадира по поводу бесплатных секунд
        int minutes = (seconds_client_stops + 30) / 60;
        minutes -= 2; // 2 минуты из насчитанных остановок не идут в расчет
        if (minutes < 0)
            minutes = 0;
        return minutes;
    }
	int secondsClientStops() { return seconds_client_stops; }
	int minutesStops() { return (seconds_stops + 30) / 60; }
	int secondsStops() { return seconds_stops; }
	int minutesTotal() { return (_total_travel_time_seconds + 30) / 60; }
	int secondsTotal() { return _total_travel_time_seconds; }
	int minutesMoving() { return (seconds_moving + 30) / 60; }
	int secondsMoving() { return seconds_moving; }
	
	bool isClientStop() { return _clientStop; }
	bool isTrainCross() { return _trainCross; }

	bool isStarted() { return started; }

	QString address() { return _address; }
	void setAddress(QString address) { _address = address; }

	void startTotalTime() { _totalTimeStarted = true; }

signals:
	void paymentChanged(int);
	void regionChanged(int regionId);
	void newMileage(float mileage);
	void newTimeMovement(int seconds);
	void newTimeStops(int seconds);
	void newTimeClientStops(int seconds);
	void newTimeTotal(int seconds);

	void movementStartFiltered(bool);

    void bonusRide(bool isBonus);

public slots:
	void recalcSum();
	void newPosition(QGeoCoordinate);
	void startOrder();
	void stopOrder();
	void measureTimes();
	void movementStart(int startStop);
	void setOutOfCity(bool _outOfCity) {
		outOfCity = _outOfCity;
	}
	void setMg(float out_of_city_rate)
	{
		_out_of_city_rate = out_of_city_rate;
	}

private:	
	// IMPORTANT ORDER VARIABLES (USED FOR MONEY COMPUATION)	
	TaxiRatePeriod taxiRate;

	bool _is_talon;

    bool _is_bonus;
    int _bonus_seconds;

	QString _address;
	
	// стоимость километра в межгороде
	float _out_of_city_rate;
	// meters in da city
	float _mileage_city;
	// meters out of city
	float _mileage_out_of_city;

	float _mileage_city_overload;
	float _mileage_out_of_city_overload;
	
	// seconds, общее время поездки
	int _total_travel_time_seconds;

	// наш id для базы
	int _order_id;
	// куда едем, id
	int _destination_region_id;
	
	// true -- идёт счёт, false -- не идёт
	bool started;

	// общее время в движении
	int seconds_moving;

	// время в пробках
	int seconds_stops;

	// остановка по просьбе водителей
	int seconds_traincross_stops;

	int seconds_client_stops;
	
	float parkingCost; 
	
	int parkingId;

	// END IMPORTANT ORDER VARIABLES
	

	QString addressText;
	
	QTimer *paymentTimer;
	
	QGeoCoordinate currentPosition;
	bool gotPosition;


	int current_stop_seconds;

	bool outOfCity;

	bool movementStarted;

	bool _overload;

	bool _clientStop;

	bool _trainCross;

	bool _totalTimeStarted;
};

#endif // TAXIORDER_H
