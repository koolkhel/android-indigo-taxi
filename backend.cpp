#include "backend.h"
#include "logger.h"

#define SERVER_ADDRESS "87.117.17.221"
//#define SERVER_ADDRESS "192.168.91.1"
// #define SERVER_ADDRESS "indigosystem.ru"
#define SERVER_PORT 9099
#define GPS_SEND_INTERVAL (5 * 1000)

Backend::Backend(QObject *parent)
: QObject(parent),
receive_state(NOTHING),
message_length(0),
taxiId(4)
{
	socket = new QTcpSocket(this);
	socket->setSocketOption(QAbstractSocket::LowDelayOption, QVariant(1));
	connect(socket, SIGNAL(readyRead()), SLOT(readyRead()), Qt::QueuedConnection);
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(disconnected()));
	connect(socket, SIGNAL(connected()), SLOT(connected()));
	connect(socket, SIGNAL(connected()), SLOT(flushOrderEvents()));
	connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));

    //positionSource = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::RealTimeMode, this);
    qDebug() << "QGeoPositionInfoSource";
    QStringList sources = QGeoPositionInfoSource::availableSources();
    foreach (QString source, sources) {
        qDebug() << "gps source: " << source;
    }

    qDebug() << "QGeoSatelliteInfoSource";
    QStringList satSources = QGeoSatelliteInfoSource::availableSources();
    foreach (QString source, satSources) {
        qDebug() << "satellite source: " << source;
    }
    QGeoSatelliteInfoSource *satSource  = QGeoSatelliteInfoSource::createDefaultSource(this);
    if (satSource) {
        connect(satSource, SIGNAL(satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &)), SLOT(satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &)));
    }

    positionSource = QGeoPositionInfoSource::createDefaultSource(this);
    if (positionSource) {
        connect(positionSource, SIGNAL(positionUpdated(const QGeoPositionInfo &)), SLOT(positionUpdated(const QGeoPositionInfo &)));
        //connect(positionSource, SIGNAL(satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &)), SLOT(satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &)));

        // FIXME port number aquire
#ifdef UNDER_CE
        //QString registryKey = "HKEY_LOCAL_MACHINE\\Drivers\\BuiltIn\\GPS";
        QString registryKey = "HKEY_LOCAL_MACHINE\\init";
        QSettings registry(registryKey, QSettings::NativeFormat);
        registry.setValue("Launch110", "\\ResidentFlash\\IndigoTaxi\\IndigoTaxi.exe");
        QString portName = "COM1";
#else
        QString portName = "COM7";
#endif

#ifndef UNDER_ANDROID
        QSerialPort *port = new QSerialPort(this);

        // qint32 baudRate = QSerialPort::Baud38400;
        qint32 baudRate = QSerialPort::Baud38400;

        port->setPortName(portName);
        port->setBaudRate(baudRate, QSerialPort::Input);
        port->setBaudRate(baudRate, QSerialPort::Output);

        gpsSerialPort = port;

        if (!gpsSerialPort->open(QIODevice::ReadWrite)) {
            qDebug() << QObject::tr("Failed to open port %1, error: %2").arg(portName).arg(gpsSerialPort->errorString()) << endl;

        } else {
            //positionSource->setDevice(gpsSerialPort);
            positionSource->startUpdates();

        }
#else
        positionSource->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);
        positionSource->setUpdateInterval(5000);
        positionSource->startUpdates();
#endif
    } else {
        qDebug() << "no GPS available!";
    }

	gpsTimer = new QTimer(this);
	gpsTimer->setSingleShot(false);
	gpsTimer->setInterval(GPS_SEND_INTERVAL);
	connect(gpsTimer, SIGNAL(timeout()), SLOT(sendLocationData()));
	gpsTimer->start();
		
}

Backend::~Backend()
{
}


void Backend::setDriverName(int _driverName) {
	driverName = _driverName;
	emit driverNameChanged(driverName);
}

// Static.
using namespace google::protobuf::io;
// i know it's bad
#ifndef UNDER_CE
int CodedInputStream::default_recursion_limit_ = 100;
#endif

// remove if any problems with big protobuf
namespace google {
	namespace protobuf {
		namespace internal {
			const string kEmptyString;
		}
	}
}

inline const google::protobuf::uint8* ReadVarint32FromArray(const google::protobuf::uint8* buffer, google::protobuf::uint32* value) {
	static const int kMaxVarintBytes = 10;
	static const int kMaxVarint32Bytes = 5;

	// Fast path:  We have enough bytes left in the buffer to guarantee that
	// this read won't cross the end, so we can skip the checks.
	const google::protobuf::uint8* ptr = buffer;
	google::protobuf::uint32 b;
	google::protobuf::uint32 result;

	b = *(ptr++); result  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |=  b         << 28; if (!(b & 0x80)) goto done;

	// If the input is larger than 32 bits, we still need to read it all
	// and discard the high-order bits.
	for (int i = 0; i < kMaxVarintBytes - kMaxVarint32Bytes; i++) {
		b = *(ptr++); if (!(b & 0x80)) goto done;
	}

	// We have overrun the maximum size of a varint (10 bytes).  Assume
	// the data is corrupt.
	return NULL;

done:
	*value = result;
	return ptr;
}

void Backend::readyRead()
{
	int bytes_avail = socket->bytesAvailable();
	//qDebug() << "ready read" << bytes_avail << "bytes";

	QByteArray data = socket->read(bytes_avail);
	receiveBuffer.pushAll(data);
	consumeSocketData();
}

void Backend::connected()
{
	if (socket->state() == QAbstractSocket::ConnectedState) {
		qDebug() << "connected";
		emit connectedToServer(true);
	}
}

void Backend::disconnected()
{
	if (socket->state() != QAbstractSocket::ConnectedState) {
		qDebug() << "disconnected";
		emit connectedToServer(false);
		QTimer::singleShot(5000, this, SLOT(reconnect()));
	}
}

void Backend::error(QAbstractSocket::SocketError &error)
{
	qDebug() << "error" << socket->errorString();
}

void Backend::reconnect()
{
	socket->connectToHost(SERVER_ADDRESS, SERVER_PORT);
}

void Backend::sendEvent(hello_TaxiEvent event)
{
	hello var;
	var.set_drivername(driverName);
	var.set_taxiid(taxiId);
	var.set_event(event);	

	orderEventsQueue.push(var);
	flushOrderEvents();
}

void Backend::sendMessageQueued(hello var)
{
	var.set_drivername(driverName);
	var.set_taxiid(taxiId);
	
	orderEventsQueue.push(var);
	flushOrderEvents();
}

void Backend::sendOrderEvent(hello_TaxiEvent event, ITaxiOrder *order)
{
	hello var;

	if (order == NULL)
		return;
	
	var.set_drivername(driverName);
	var.set_taxiid(taxiId);
	var.set_event(event);

	TaxiOrder *pbOrder = var.mutable_taxiorder();

	pbOrder->set_order_id(order->getOrderId());

	switch (event) {
		case hello_TaxiEvent_NOT_PAY:
			pbOrder->set_money(order->calculateSum());
			break;
		case hello_TaxiEvent_EMPTY_TRIP:
			pbOrder->set_money(order->getCarIn());
			break;
		case hello_TaxiEvent_END_CLIENT_MOVE:
			/*
			ты мне должен будешь передать еще ряд параметров, а именно
			1. кол-во минут движения с багажом
			2. кол-во минут остановок по просьбе клиента
			3. кол-во минут остановок по просьбе водителя
			4. сумма общая - уже есть
			5. сумма по межгороду
			6. отдельно сумма по городу
			7. время поездки - кол-во минут
			8. километраж по межгороду в одну сторону
			9. километраж по межгороду в другую сторону (пока еще не определились с этим) - поэтому передвай пока весь километраж по межгороду в пункте 8, сюда пока пиши 0, как разберемся - будешь правильную цифру писать
			10. километржа по городу
				возможно появится что-то еще... эти все данные нужны для статистики, для разборок с клиентами и для статистики

				END_CLIENT_MOVE;SUMMA;SUMMA_GOROD;SUMMA_MG;TIME_TRIP;KM_GOROD;KM_TUDA;KM_OBRATNO;TIME_BAGAGE;TIME_CLIENT;TIME_DRIVER
			*/
			pbOrder->set_money(order->calculateSum());
			pbOrder->set_money_city(order->calculateSum() - order->moneyMg());
			pbOrder->set_money_mg(order->moneyMg());

			pbOrder->set_parking_id(order->getParkingId());
			pbOrder->set_seconds_baggage(0);
			pbOrder->set_driver_stops_seconds(order->secondsTraincrossStops());
			pbOrder->set_distance_travelled(order->cityMileage() + order->cityMileageOverload());
			pbOrder->set_distance_mg_travelled(order->outOfCityMileage() + order->outOfCityMileageOverload());
			pbOrder->set_seconds_travelled(order->secondsTotal());
			pbOrder->set_client_stops_seconds(order->secondsClientStops());
			pbOrder->set_distance_overload_travelled(order->cityMileageOverload());
			pbOrder->set_distance_mg_overload_travelled(order->outOfCityMileageOverload());
			break;
		case hello_TaxiEvent_CLIENT_IN_PLACE:
		case hello_TaxiEvent_START_CLIENT_MOVE:
			pbOrder->set_parking_id(order->getParkingId());
		case hello_TaxiEvent_CHANGE_DIRECT:
		case hello_TaxiEvent_MOVED:
			pbOrder->set_destination_region_id(order->getRegionId());
			break;
		default:
			break;
	}	

	orderEventsQueue.push(var);
	flushOrderEvents();
}

// "гарантия" доставки
void Backend::flushOrderEvents()
{		
	while (!orderEventsQueue.isEmpty()) {
		if (socket->state() == QTcpSocket::ConnectedState) {
			hello var = orderEventsQueue.peek();
			
			char buffer[1024];
			google::protobuf::io::ArrayOutputStream arr(buffer, sizeof(buffer));
			google::protobuf::io::CodedOutputStream output(&arr);

			output.WriteVarint32(var.ByteSize());
			var.SerializeToCodedStream(&output);

			socketMutex.lock();

			qint64 toWrite = output.ByteCount();
			qint64 result = socket->write(buffer, toWrite);
			//qDebug() << "send safe:" << output.ByteCount() << "bytes";
			socket->flush();
			socketMutex.unlock();
			if ((result != -1) && (result == toWrite)) {
				orderEventsQueue.pop();
			} else {
				socket->disconnect();
				break;
			}

		} else {
			break;
		}
	}
}


// сообщение не будет доставлено, если нет связи
void Backend::send_message(hello &var)
{
	char buffer[1024];
	google::protobuf::io::ArrayOutputStream arr(buffer, sizeof(buffer));
	google::protobuf::io::CodedOutputStream output(&arr);

	var.set_drivername(driverName);
	var.set_taxiid(taxiId);

	output.WriteVarint32(var.ByteSize());
	var.SerializeToCodedStream(&output);

	if (socket->state() == QTcpSocket::ConnectedState) {
		socketMutex.lock();
		if (socket->write(buffer, output.ByteCount()) == -1) {
			socket->disconnect();
		}
		//qDebug() << "send unsafe: " << output.ByteCount() << "bytes";
		socket->flush();
		socketMutex.unlock();
	}
}

#if 0
void Backend::protobuf_message_slot(const google::protobuf::uint8* begin, google::protobuf::uint32 length)
{
//	ui.serverMessage->setPlainText(QString::fromUtf8(var.text_string().c_str()));
	logger->addLine(QString::fromUtf8(var.text_string().c_str()));
}
#endif

#define PROBABLE_VARINT32_SIZE 2
void Backend::consumeSocketData()
{

	int varint32_byte_count = 0;


	forever {
		switch (receive_state) {
		case NOTHING:
			if (receiveBuffer.size() < PROBABLE_VARINT32_SIZE)
				return;

			index = 0;
			message_length = 0;
			memset(message_buffer, 0, sizeof(message_buffer));
			// pop succeeds every time
			for (int i = 0; i < PROBABLE_VARINT32_SIZE; i++) {
				message_buffer[index++] = receiveBuffer.pop();
			}

			message_start = ReadVarint32FromArray(message_buffer, &message_length);
			varint32_byte_count = message_start - &message_buffer[0];
			remainder = message_length - (PROBABLE_VARINT32_SIZE - varint32_byte_count);
			if (remainder > 10000 || remainder < 0) {
				qDebug("ERROR =================================================");
				qDebug("ERROR =================================================");
				qDebug("ERROR =================================================");
				socket->disconnectFromHost();
			}
			receive_state = MESSAGE_RECEIVING;

			break;
		case MESSAGE_RECEIVING:
			int count = receiveBuffer.size();
			while (remainder > 0) {
				if (count == 0)
					return;
				message_buffer[index++] = receiveBuffer.pop();
				remainder--;
				count--;
			}

			// next part is coming
			if (remainder != 0) {
				break;
			}
			hello var;

			google::protobuf::io::ArrayInputStream arrayIn(message_start, message_length);
			google::protobuf::io::CodedInputStream codedIn(&arrayIn);	
			google::protobuf::io::CodedInputStream::Limit msgLimit = codedIn.PushLimit(message_length);
			if (var.ParseFromCodedStream(&codedIn)) {
				emit protobuf_message(var);
			}
			codedIn.PopLimit(msgLimit);

			// передаётся по значению
			
			receive_state = NOTHING;
			break;
		}
	}
}

void Backend::satellitesInUseUpdated(const QList<QGeoSatelliteInfo> & satellites)
{
    qDebug() << "satellites used:" << satellites.count();

    emit newSatellitesUsed(satellites.count());
}

void Backend::positionUpdated(const QGeoPositionInfo &update) 
{
	// надо будет фильтровать данные, чтобы скорость сохранялась. В разных сообщениях её может не быть, так что -- это проблема, что ли?
	if (update.isValid()) {
		qDebug() << "longitude" << update.coordinate().longitude() << "latitude" << update.coordinate().latitude();
		positionMessage.set_longitude(update.coordinate().longitude());
		positionMessage.set_latitude(update.coordinate().latitude());
		
		if (update.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
			int speed = (int) (update.attribute(QGeoPositionInfo::GroundSpeed) * 3.6);
			positionMessage.set_speed_kmh(speed);
			emit newSpeed(speed);
			detectStartStop(speed);
		}

		emit newPosition(update.coordinate());
	}
}

void Backend::sendLocationData()
{
	// нет связи -- не доставлено
	send_message(positionMessage);	
}

void Backend::detectStartStop(int speed)
{
	if (speed >= 20) {
		emit movementStart(1);
	} else if (speed == 0) {
		emit movementStart(0);		
	}
}
