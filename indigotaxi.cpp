#include "indigotaxi.h"
#include <QtGui>
#include <QtCore>
#include <QDesktopWidget>

#ifndef UNDER_ANDROID
#include "windows.h"
#endif
#ifdef UNDER_CE
// для выключенияf
#include "Pm.h"
#endif

#include "backend.h"
#include "voicelady.h"

/* main version string! */
static const char *version = "0.1.028";
int const IndigoTaxi::EXIT_CODE_REBOOT = -123456789;

IndigoTaxi::IndigoTaxi(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), iTaxiOrder(NULL), lastTaxiOrder(NULL), 
	satellitesUsed(0), movementStarted(false), currentParkingCost(0), currentParkingId(0),
	newDirection(false), online(false), downloader(NULL), changeRegion(false), asked_region_id(0),
	_taxiRateUpdated(false), _taxiRateReceived(false), _updatePerformed(false), _intercity(0),
	_stop_sound_played(false), _start_sound_played(false), _driverOrder(0), colorTheme(INDIGO_DARK_THEME),
	_changeRegionStopEvent(hello_TaxiEvent_NOTHING), _dpi(120), _width(800), _height(480), _orderOfferGuard(false)
{
	ui.setupUi(this);
#ifdef UNDER_CE
    QWindowFlags flags = 0;
    flags = Qt::Window | Qt::FramelessWindowHint;
    setWindowFlags(flags);
#endif
	setAttribute(Qt::WA_QuitOnClose);

	//settingsForm = new SettingsForm(this);
	//settingsForm->hide();
	infoDialog = new IInfoDialog(this);
	confirmDialog = new IConfirmationDialog(this);
	driverNumberDialog = new DriverNumberDialog(this);

	downloadManager = new DownloadManager(this);

	QFile::remove("download.complete");
	QFile::remove("download.part");

	backend = new Backend(this);
	
	qRegisterMetaType<hello>("hello");
	connect(backend, SIGNAL(protobuf_message(hello)), SLOT(protobuf_message(hello)), Qt::QueuedConnection);
	connect(backend, SIGNAL(connectedToServer(bool)), SLOT(connectionStatus(bool)));
	connect(backend, SIGNAL(driverNameChanged(int)), SLOT(driverNameChanged(int)));
	connect(backend, SIGNAL(newSpeed(int)), SLOT(newSpeed(int)));
	connect(backend, SIGNAL(newSatellitesUsed(int)), SLOT(newSatellitesUsed(int)));
	connect(backend, SIGNAL(movementStart(int)), SLOT(movementStart(int)));
	//settingsForm->setBackend(backend);

	settingsIniFile = new QSettings("indigotaxi.ini", QSettings::IniFormat, this);
	settingsIniFile->beginGroup("main");
	int driverName = settingsIniFile->value("driverName", QVariant(500)).toInt();
	settingsIniFile->endGroup();
	backend->setDriverName(driverName);
	backend->sendEvent(hello_TaxiEvent_GET_INFO);

	//ui.versionLabel->setText(version);

	connect(this, SIGNAL(reboot_application()), SLOT(rebootApp()));

	updateStartTimer = new QTimer(this);
	connect(updateStartTimer, SIGNAL(timeout()), SLOT(updatesDownloadTipVersionString()));
	updateStartTimer->setInterval(10 * 1000);
	updateStartTimer->setSingleShot(true);

	updateDownloadTimeoutTimer = new QTimer(this);
	connect(updateDownloadTimeoutTimer, SIGNAL(timeout()), SLOT(updateDownloadTimeout()));
	updateDownloadTimeoutTimer->setSingleShot(true);
	// 15 seconds
	updateDownloadTimeoutTimer->setInterval(15 * 3000);

	timeTimer = new QTimer(this);
	connect(timeTimer, SIGNAL(timeout()), SLOT(updateTime()));
	timeTimer->setInterval(1000);
	timeTimer->setSingleShot(false);
	timeTimer->start();

	connectedTimer = new QTimer(this);
	connect(connectedTimer, SIGNAL(timeout()), SLOT(connectedTimerTimeout()));
	connectedTimer->setInterval(5000);
	connectedTimer->setSingleShot(true);

	_driverOrderUpdateTimer = new QTimer(this);
	connect(_driverOrderUpdateTimer, SIGNAL(timeout()), SLOT(driverUpdateTimerTimeout()));
	_driverOrderUpdateTimer->setInterval(20000);
	_driverOrderUpdateTimer->setSingleShot(false);
	_driverOrderUpdateTimer->start();
	connect(this, SIGNAL(driverOrderUpdated(int)), SLOT(driverOrderUpdatedSlot(int)));


	voiceLady = new VoiceLady(this);
	iSoundPlayer = new ISoundPlayer();
	soundThread = new QThread(this);
	soundThread->start();

	iSoundPlayer->moveToThread(soundThread);

	connect(voiceLady, SIGNAL(playSound(QString)), iSoundPlayer, SLOT(playResourceSound(QString)));
	connect(voiceLady, SIGNAL(playSoundFile(QString)), iSoundPlayer, SLOT(playFileSystemSound(QString)));

	//ui.driverNameLineEdit->setProperty("keyboard",true); // enable the keyboard. when there is no validator set the keyboard will show
	//aTextLineEdit->setProperty("maxLength",25); //this can be used to limit the length of the string
	//int dpi = 122;
    _dpi = 170;
	QRect rect = QApplication::desktop()->geometry();
	_width = rect.width();
	_height = rect.height();

#ifdef UNDER_CE
	
	_dpi = (int) sqrt(_width*_width + _height*_height) / 4.5; // average screen size
	qDebug() << "calculated DPI:" << _dpi;
#endif
	orderReceiveTimer = new QTimer(this);
	orderReceiveTimer->setSingleShot(false);
	connect(orderReceiveTimer, SIGNAL(timeout()), SLOT(orderReceiveTimerTimeout()));

	setProperty("_q_customDpiX", QVariant(_dpi));
	setProperty("_q_customDpiY", QVariant(_dpi));
	
	confirmDialog->setProperty("_q_customDpiX", QVariant(_dpi));
	confirmDialog->setProperty("_q_customDpiY", QVariant(_dpi));
	confirmDialog->setMinimumSize((int) _width * 0.8, (int) _height * 0.9);
	confirmDialog->setMaximumSize((int) _width * 0.8, (int) _height * 0.9);
	
	driverNumberDialog->setProperty("_q_customDpiX", QVariant(_dpi));
	driverNumberDialog->setProperty("_q_customDpiY", QVariant(_dpi));
	driverNumberDialog->setMinimumSize((int) _width * 0.8, (int) _height * 0.9);

	ui.regionList->setProperty("_q_customDpiX", QVariant(_dpi));
	ui.regionList->setProperty("_q_customDpiY", QVariant(_dpi));

	ui.regionListSettingsWidget->setProperty("_q_customDpiX", QVariant(_dpi));
	ui.regionListSettingsWidget->setProperty("_q_customDpiY", QVariant(_dpi));
	
	ui.regionDetailsList->setProperty("_q_customDpiX", QVariant(_dpi));
	ui.regionDetailsList->setProperty("_q_customDpiY", QVariant(_dpi));
	
	ui.messageTemplatesList->setProperty("_q_customDpiX", QVariant(_dpi));
	ui.messageTemplatesList->setProperty("_q_customDpiY", QVariant(_dpi));

	ui.taxiRateTableWidget->setProperty("_q_customDpiX", QVariant(_dpi));
	ui.taxiRateTableWidget->setProperty("_q_customDpiY", QVariant(_dpi));

	ui.settingsTabWidget->setProperty("_q_customDpiX", QVariant(_dpi));
	ui.settingsTabWidget->setProperty("_q_customDpiY", QVariant(_dpi));

	int tab_width = _width / ui.settingsTabWidget->count() - 3;
	//int tab_width = 197;
	int tab_height = (int) _height * 0.15;
	//de	int tab_height = 120;
	ui.settingsTabWidget->setStyleSheet(QString("QTabBar::tab { width: %1px; height: %2px;}").arg(tab_width).arg(tab_height));

	setCurrentScreenFromSettings();

	progressDialog = new QProgressDialog("Получение информации о районе", "Отмена", 0, 2, this);

	ui.orderHistoryTable->verticalHeader()->setVisible(false);
    ui.orderHistoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.orderHistoryTable->setRowCount(0);

    ui.taxiRateTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.taxiRateTableWidget->resizeColumnsToContents();

    ui.messageHistoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.messageHistoryTable->verticalHeader()->setVisible(false);

	applyColorTheme();

	backend->reconnect();

	ui.stackedWidget->setCurrentWidget(ui.settingsPage4);
	ui.settingsTabWidget->setCurrentWidget(ui.driverCabinetSettingsTab2);
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPage1);
}
		
IndigoTaxi::~IndigoTaxi()
{

}

void IndigoTaxi::changeBackLightClicked()
{
	bool onOff = true;
	
	QString registryKey = "HKEY_CURRENT_USER\\ControlPanel\\BackLight";
	QSettings registry(registryKey, QSettings::NativeFormat);
	
	int value = (registry.value("ACPrescale", 100)).toInt();
	
	onOff = value == 100;

	backlight(!onOff);
}

void IndigoTaxi::backlight(bool onOff)
{
	int value = onOff ? 100 : 0;
	
	QString registryKey = "HKEY_CURRENT_USER\\ControlPanel\\BackLight";
	QSettings registry(registryKey, QSettings::NativeFormat);
	
	registry.setValue("ACPrescale", value);
	registry.setValue("BatteryPrescale", value);
	
	registry.sync();

#ifdef UNDER_CE
	HANDLE hBackLightEvent = CreateEvent( NULL, FALSE, TRUE, TEXT("BackLightChangeEvent")); 

	if (hBackLightEvent) { 
		SetEvent(hBackLightEvent);
		CloseHandle(hBackLightEvent); 
	}
#endif

	//rebootSystem();
}

void IndigoTaxi::changeDriverNumberClicked()
{
	if (driverNumberDialog->showPassword()) {
		backend->setDriverName(driverNumberDialog->driverNumber());
		infoDialog->info("ПОЗЫВНОЙ СМЕНЁН");
	} else {
		infoDialog->info("ОШИБКА: НЕВЕРНЫЙ ПАРОЛЬ");
	}
}

void IndigoTaxi::setCurrentScreenFromSettings()
{
	QString status = getSettingsStatus();

	if (status == "OK") {
	} else if (status == "DINNER") {
		ui.stackedWidget->setCurrentWidget(ui.settingsPage4);
		enableDutyUI(true);
		ui.settingsTabWidget->setCurrentWidget(ui.driverCabinetSettingsTab2);
		ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageDinner2);			
	} else if (status == "FROMCAR") {
		ui.settingsTabWidget->setCurrentWidget(ui.driverCabinetSettingsTab2);
		ui.stackedWidget->setCurrentWidget(ui.settingsPage4);
		enableDutyUI(true);
		ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageFromcar3);
	} else if (status == "AWAY") {
		ui.settingsTabWidget->setCurrentWidget(ui.driverCabinetSettingsTab2);
		ui.stackedWidget->setCurrentWidget(ui.settingsPage4);
		enableDutyUI(true);
		ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageAway4);
	} else if (status == "REPAIR") {
		ui.settingsTabWidget->setCurrentWidget(ui.driverCabinetSettingsTab2);
		ui.stackedWidget->setCurrentWidget(ui.settingsPage4);
		enableDutyUI(true);
		ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageRepair5);
	} else if (status == "TECHHELP") {
		ui.settingsTabWidget->setCurrentWidget(ui.driverCabinetSettingsTab2);
		ui.stackedWidget->setCurrentWidget(ui.settingsPage4);	
		enableDutyUI(true);
		ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageTechhelp6);
	}
}

void IndigoTaxi::driverOrderUpdatedSlot(int driverNumber)
{
	voiceLady->click();
	_driverOrder = driverNumber;

	if (driverNumber == 0) {
		ui.taxiDriverOrderNumberLabel->setText("");
	} else {
		ui.taxiDriverOrderNumberLabel->setText(QString("[%1]").arg(driverNumber));
	}
}

void IndigoTaxi::driverUpdateTimerTimeout()
{
	backend->sendEvent(hello_TaxiEvent_GET_DRIVER_ORDER);
}

void IndigoTaxi::updateTime()
{
	QDateTime dateTime = QDateTime::currentDateTimeUtc();	
	QTime time = dateTime.addSecs(3 * 3600).time(); // MSK+3
	QString text = time.toString("hh:mm");
    ui.timeLabel->setText(text);

	// если сразу не получили (не было интернета), но очень хочется
	if (!_taxiRateReceived && (time.second() == 30 || time.second() == 0)) {
		backend->sendEvent(hello_TaxiEvent_GET_INFO);
	}

	if (!_taxiRateUpdated || time.second() == 0) {
		updateTaxiRates();
		_taxiRateUpdated = true;
	}
}

void IndigoTaxi::settingsButtonClick()
{
	//emit reboot_application();
	//rebootApp();
	ui.stackedWidget->setCurrentWidget(ui.settingsPage4);
}

void IndigoTaxi::moveToClient() 
{
	backend->sendOrderEvent(hello_TaxiEvent_MOVE_TO_CLIENT, iTaxiOrder);
	voiceLady->sayPhrase("ORDERACCEPT");
	orderReceiveTimer->stop(); // сбрасываем таймаут на приём заказа
}

void IndigoTaxi::inPlace()
{
	backend->sendOrderEvent(hello_TaxiEvent_IN_PLACE, iTaxiOrder);
	voiceLady->sayPhrase("ORDERINPLACE");
}

void IndigoTaxi::driverRegionSelectClicked()
{
	changeRegion = true;

	ui.stackedWidget->setCurrentWidget(ui.regionListPage6);
}

void IndigoTaxi::startClientMoveClicked()
{
	//voiceLady->sayPhrase("ORDERGO");
}

void IndigoTaxi::startClientMove()
{	
	// смена района таксиста
	if (iTaxiOrder == NULL && changeRegion)
	{
		hello var;
		TaxiOrder *pbOrder = var.mutable_taxiorder();
		pbOrder->set_order_id(NO_ORDER_ID);
		pbOrder->set_destination_region_id(taxiRegionList.regions().Get(ui.regionList->currentRow()).region_id());
		var.set_event(hello_TaxiEvent_MOVED);
		backend->sendMessageQueued(var);
		changeRegion = false;
		ui.stackedWidget->setCurrentWidget(ui.standByPage1);
		return;
	}

	// если заказ инииирован на месте
	if (iTaxiOrder == NULL) {
		// заказ инициирован водителем
		iTaxiOrder = createTaxiOrder(NO_ORDER_ID);
	}

	float stopsTaxiRate = (floor((iTaxiOrder->orderTaxiRate().car_min() / 2.0) * 10)) / 10.0;
	float clientStopsTaxiRate = (floor((iTaxiOrder->orderTaxiRate().client_stop()) * 10)) / 10.0;
	ui.stopsTaxiRateLabel->setText(QString("%1")
		.arg(clientStopsTaxiRate, 0, 'f', 1));

	float kmTaxiRate = iTaxiOrder->orderTaxiRate().km_g();
	float kmgTaxiRate = iTaxiOrder->mgRate();
	ui.kmgTaxiRateLabel->setText(QString("%1/%2")
		.arg(kmTaxiRate, 0, 'f', 1)
		.arg(kmgTaxiRate, 0, 'f', 1));

	float overloadCityTaxiRate = kmTaxiRate * 1.5;
	float overloadOutOfCityTaxiRate = kmgTaxiRate * 1.5;
	ui.overloadTaxiRateLabel->setText(QString("%1/%2")
		.arg(overloadCityTaxiRate, 0, 'f', 1)
		.arg(overloadOutOfCityTaxiRate, 0, 'f', 1));


	float carInRate = iTaxiOrder->orderTaxiRate().car_in() + currentParkingCost;
	ui.finalCarInLabel->setText(QString("%1").arg(carInRate, 0, 'f', 1));
	ui.finalCarInTotalLabel->setText(QString("%1").arg(carInRate, 0, 'f', 1));

	// целевой регион
	iTaxiOrder->setRegionId(taxiRegionList.regions().Get(ui.regionList->currentRow()).region_id());
	// сообщаем серверу
	if (newDirection) {
		backend->sendOrderEvent(hello_TaxiEvent_CHANGE_DIRECT, iTaxiOrder);
	} else {
		if (iTaxiOrder->getOrderId() == NO_ORDER_ID) {
			backend->sendOrderEvent(hello_TaxiEvent_CLIENT_IN_PLACE, iTaxiOrder);
		} else
		{
			backend->sendOrderEvent(hello_TaxiEvent_START_CLIENT_MOVE, iTaxiOrder);
		}
		
		// пошёл счёт
		iTaxiOrder->startOrder();
		iTaxiOrder->startTotalTime();
	}
	
	// обновляем цифры
	iTaxiOrder->recalcSum();
	// на экране заказа пишем район, куда едем
	ui.directionValueButton->setText("НАПРАВЛЕНИЕ\n" +
		QString::fromUtf8(taxiRegionList.regions().Get(ui.regionList->currentRow()).region_name().c_str()).toUpper());
	// экран заказа
	ui.stackedWidget->setCurrentWidget(ui.orderPage2);

	if (!newDirection) {
		voiceLady->sayPhrase("GREETING");
	}

	newDirection = false;
}

// с сервера нам что-то пришло, надо реагировать
void IndigoTaxi::protobuf_message(hello message)
{
	if (message.event() == hello_TaxiEvent_YES_GO_DINNER || message.event() == hello_TaxiEvent_NO_GO_DINNER) {
		dinnerHandleAnswer(message);
		return;
	}
	
	// старый способ доставки адреса
	if (message.event() == hello_TaxiEvent_ABORT_ORDER) {
		abortOrder(message.taxiorder().order_id());
		return;
	}

	if (message.event() == hello_TaxiEvent_ORDER_OFFER) {
		handleOrderOffer(message);
		return;
	}

	if (message.event() == hello_TaxiEvent_GET_DRIVER_ORDER) {
		qDebug() << "new driver sequence number";
		if (message.driverinfo().region_order() != _driverOrder) {
			emit driverOrderUpdated(message.driverinfo().region_order());
		}
		return;
	}

	if (message.has_taxicount()) {
		handleTaxiCount(message);
		return;
	}

	if (message.event() == hello_TaxiEvent_PERSONAL_ANSWER) {
		handlePersonalAnswer(message);
		return; // криво, но не надо спускать в новый заказ
	}

	if (message.event() == hello_TaxiEvent_TEXT_MESSAGE) {
		handleTextMessage(message);
	}
#if 0
	if (message.text_string().length() > 0)
		ui.serverMessage->setPlainText(QString::fromUtf8(message.text_string().c_str()));
#endif
	// новый способ доставки адреса
	if (message.has_taxiorder()) {
		handleNewOrder(message.taxiorder());		
	}

	if (message.has_taximessagetemplates()) {
		taxiMessageTemplates = message.taximessagetemplates();
		handleNewMessageTemplates();
	}

	if (message.has_taxirate()) {
		_taxiRateReceived = true;
		taxiRates = message.taxirate();
		updateTaxiRates();
	}

	if (message.has_taxiregionlist()) {
		// почему-то (?) иногда приходят кривые сообщения
		if (message.taxiregionlist().regions_size() > 0) {
			taxiRegionList = message.taxiregionlist();
			updateTaxiRegionList();
		}
	}

	if (message.has_taxiinfo()) {
		taxiInfo = message.taxiinfo();
		updateTaxiInfo();
	}

	if (message.has_taxiregioninfo() && message.event() == hello_TaxiEvent_ASK_REGION) {
		processAskRegionReply(message);
	}
}

void IndigoTaxi::handleOrderOffer(hello var) {
	QString address = QString::fromUtf8(var.taxiorder().address().c_str());
	hello answer;
	
	if (!_orderOfferGuard) {
		_orderOfferGuard = true;
		voiceLady->sayPhrase("MESSAGERECEIVED");
		voiceLady->click();
		voiceLady->click();
		voiceLady->click();
	
		if (confirmDialog->askYesNo("Адрес " + address + ". Заберешь по освобождению?")) {
			answer.set_event(hello_TaxiEvent_YES);
		} else {
			answer.set_event(hello_TaxiEvent_NO);
		}
	
		TaxiOrder *order = answer.mutable_taxiorder();
		order->set_order_id(var.taxiorder().order_id());

		backend->send_message(answer);
		_orderOfferGuard = false;
	}
}

void IndigoTaxi::handleNewMessageTemplates()
{
	int count = taxiMessageTemplates.templates_size();

	ui.messageTemplatesList->clear();
	for (int i = 0; i < count; i++) {
		QString message = QString::fromUtf8(taxiMessageTemplates.templates().Get(i).c_str());
		ui.messageTemplatesList->addItem(message);
	}
}

void IndigoTaxi::showInfoDialog(QString message)
{
	IInfoDialog *infoDialog = new IInfoDialog(this);
	
	infoDialog->setProperty("_q_customDpiX", QVariant(_dpi));
	infoDialog->setProperty("_q_customDpiY", QVariant(_dpi));
	infoDialog->setMinimumSize((int) _width * 0.8, (int) _height * 0.9);

	infoDialog->info(message);
	
	delete infoDialog;
}

void IndigoTaxi::handleTextMessage(hello var)
{
	// TODO нельзя в любой момент показать сообщение
	
	QString message = QString::fromUtf8(var.text_string().c_str());
	addMessageHistory(message);
	
	voiceLady->sayPhrase("MESSAGERECEIVED");
	showInfoDialog(message);
#if 0
	if (ui.stackedWidget->currentWidget() == ui.standByPage1) {
		infoDialog->info(message);
	} else {
		_messagesToShow.append(message);
	}
#endif
}

void IndigoTaxi::addMessageHistory(QString message)
{
	ui.messageHistoryTable->insertRow(0);
	ui.messageHistoryTable->setItem(0, 0, new QTableWidgetItem(QDate::currentDate().toString()));
	ui.messageHistoryTable->setItem(0, 1, new QTableWidgetItem(message));
	ui.messageHistoryTable->resizeColumnsToContents();
}

void IndigoTaxi::messagesHistoryBackClicked()
{
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPage1);
}

void IndigoTaxi::messagesHistoryClicked()
{
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageMessages8);
}

void IndigoTaxi::stackedWidgetCurrentChanged(int pageIndex)
{
	if (pageIndex == 0) {
		// страница ожидания
		foreach (QString message, _messagesToShow) {
			infoDialog->info("СООБЩЕНИЕ ДИСПЕТЧЕРА: " + message);
		}
		_messagesToShow.clear();
	} else if (ui.stackedWidget->currentWidget()->objectName() == "settingsPage4"
		&& ui.settingsTabWidget->currentWidget()->objectName() == "regionsSettingsTab4") {
		// Вкладка районы
		backend->sendEvent(hello_TaxiEvent_GET_TAXI_COUNT);
	}
}

// вернулись с деталей по району, обновляем список машин
void IndigoTaxi::regionSettingsTabWidgetChanged(int)
{
	if (ui.regionsSettingsStackedWidget->currentWidget() == ui.regionsSettingsPage1) {
		backend->sendEvent(hello_TaxiEvent_GET_TAXI_COUNT);
	}
}

void IndigoTaxi::messagesBackClicked()
{
	ui.orderSettingsStackedWidget->setCurrentWidget(ui.orderSettingsStackedWidgetPage1);
}

void IndigoTaxi::messagesShowListClicked()
{
	ui.orderSettingsStackedWidget->setCurrentWidget(ui.orderSettingsStackedWidgetPage2);
}

void IndigoTaxi::messagesSendClicked()
{
	QString message = QString::fromUtf8(taxiMessageTemplates.templates().Get(ui.messageTemplatesList->currentRow()).c_str());
	if (confirmDialog->ask("ОТПРАВИТЬ СООБЩЕНИЕ " + message)) {
		hello var;
		
		var.set_event(hello_TaxiEvent_TEXT_MESSAGE);
		var.set_text_string(message.toUtf8());
		backend->send_message(var);
		
		infoDialog->info("СООБЩЕНИЕ ОТПРАВЛЕНО");
		voiceLady->click();		
		ui.orderSettingsStackedWidget->setCurrentWidget(ui.orderSettingsStackedWidgetPage1);
	}
}

void IndigoTaxi::handlePersonalAnswer(hello var)
{
	if (iTaxiOrder != NULL && iTaxiOrder->getOrderId() == NO_ORDER_ID) {
		qDebug() << "PERSONAL_ANSWER" << var.taxiorder().order_id();
		orderReceiveTimer->stop();
		iTaxiOrder->setOrderId(var.taxiorder().order_id());
		ui.serverMessage->setPlainText("ПЕРСОНАЛКА");
	}
}

void IndigoTaxi::processAskRegionReply(hello var)
{
	progressDialog->hide();

	regions_stops_ids.clear();	
	regions_stops_names.clear();
	ui.regionDetailsList->clear();
	// По району/0/5,2,3,36,13*;РП/59/13*,36;МАГНИТ/61/3
	QString data = QString::fromUtf8(var.taxiregioninfo().region_data().c_str());
	QStringList rows = data.split(";");
	
	qDebug() << "region row count" << rows.count();
	
	for (int i = 0; i < rows.count(); i++) {
		QStringList parts = rows[i].split("/");

		if (parts.count() != 3) {
			qDebug() << "error data";
			continue;
		}

		regions_stops_ids.append(parts[1].toInt());
		regions_stops_names.append(parts[0]);
		ui.regionDetailsList->addItem(parts[0] + ": " + parts[2]);
	}
	ui.regionDetailsList->setCurrentRow(0);

	ui.regionsSettingsStackedWidget->setCurrentWidget(ui.regionsSettingsPage2);
}

void IndigoTaxi::abortOrder(int order_id)
{
	if (iTaxiOrder != NULL && iTaxiOrder->getOrderId() == order_id) {
		QString address = iTaxiOrder->address();
		voiceLady->sayPhrase("ORDERABORT");
		
		saveOrderHistory(iTaxiOrder, ITaxiOrder::ABORT_DISPATCHER);
		//backend->sendOrderEvent(hello_TaxiEvent_ABORT_ORDER, iTaxiOrder); // чтобы цвет сменился
		
		destroyCurrentOrder();
		infoDialog->info("ЗАКАЗ НА АДРЕС " + address + " ОТМЕНЁН ДИСПЕТЧЕРОМ");
		ui.stackedWidget->setCurrentWidget(ui.standByPage1);
	}
}
	
void IndigoTaxi::intercity(int intercity)
{
	if (intercity == 1 && _intercity == 0) {
		ui.intercityLabel->setText("МЕЖГОРОД");
		_intercity = intercity;
		voiceLady->sayPhrase("INTERCITY");
	} else if (intercity == 0 && _intercity == 1) {
		ui.intercityLabel->setText("ГОРОД");
		_intercity = intercity;
		voiceLady->sayPhrase("INCITY");
	}
}

void IndigoTaxi::updateTaxiInfo()
{
	if (taxiInfo.out_of_city()) {
		intercity(1);
	
		qDebug() << "out of town";
	} else {
		intercity(0);
	
		qDebug() << "inside town" << QString::fromUtf8(taxiInfo.city_name().c_str());
	}
	
	if (taxiInfo.inside_parking()) {
		currentParkingCost = taxiInfo.parking_price();
		currentParkingId = taxiInfo.parking_id();

		qDebug() << "inside parking" << currentParkingId << "cost" << currentParkingCost;
	} else {
		currentParkingCost = 0;
		currentParkingId = 0;

		qDebug() << "outside parking";
	}
	

	if (iTaxiOrder == NULL)
		return;

	iTaxiOrder->setOutOfCity(taxiInfo.out_of_city());

}

/**
 * Обновление на экране текущего тарифа по часам
 *
 */
void IndigoTaxi::updateTaxiRates()
{
	for (int i = 0; i < taxiRates.rates_size(); i++) {
		//qDebug() << "Car In:" << taxiRates.rates().Get(i).car_in() <<
		//	"KM_G:" << taxiRates.rates().Get(i).km_g();
	}
	TaxiRatePeriod period = getCurrentTaxiRatePeriod();
	//ui.car_in_label->setText(QString("%1 руб.").arg(period.car_in() + currentParkingCost, 0, 'f', 1));
	ui.car_in_label->setText(QString("%1 руб.").arg(period.car_in()));
	ui.km_g_label->setText(QString("%1 руб.").arg(period.km_g(), 0, 'f', 1));
	ui.km_mg_label->setText(QString("%1 руб.").arg(taxiRates.mg(), 0, 'f', 1));
	
	float clientStopsTaxiRate = (floor((period.client_stop()) * 10)) / 10.0;
	float stopsTaxiRate = (floor((period.car_min() * 0.5) * 10)) / 10.0;
	ui.client_stop_label->setText(QString("%1 руб.")
		.arg(clientStopsTaxiRate, 0, 'f', 1));

	// километраж
	ui.taxiRateTableWidget->setItem(0, 2, new QTableWidgetItem(QString("%1/%2").arg(period.km_g(), 0, 'f', 1).arg(taxiRates.mg(), 0, 'f', 1)));
	// подача машины
	ui.taxiRateTableWidget->setItem(1, 2, new QTableWidgetItem(QString("%1").arg(period.car_in(), 0, 'f', 1)));
	// остановки по просьбе клиента
	ui.taxiRateTableWidget->setItem(2, 2, new QTableWidgetItem(QString("%1").arg(clientStopsTaxiRate, 0, 'f', 1)));
	// пробки
	ui.taxiRateTableWidget->setItem(4, 2, new QTableWidgetItem(QString("%1").arg(period.car_min() * 0.5	, 0, 'f', 1)));
}

/**
 * Пришли новые районы, заполняем ими таблицу с выбором 
 *
 */
void IndigoTaxi::updateTaxiRegionList()
{
	// только здесь мы уверены, что смена, наконец, началась
	ui.regionList->clear();
	ui.regionListSettingsWidget->clear();
	for (int i = 0; i < taxiRegionList.regions_size(); i++) {
		QString regionName = QString::fromUtf8(taxiRegionList.regions().Get(i).region_name().c_str());
		ui.regionList->addItem(regionName);
		ui.regionListSettingsWidget->addItem(regionName);
	}
	ui.regionList->setCurrentRow(0);
	ui.regionListSettingsWidget->setCurrentRow(0);
}

void IndigoTaxi::settingsTabWidgetChanged(int tabNumber)
{
	if (ui.settingsTabWidget->widget(tabNumber)->objectName() == "regionsSettingsTab4") {
		// Вкладка районы
		backend->sendEvent(hello_TaxiEvent_GET_TAXI_COUNT);
	}
}

void IndigoTaxi::handleTaxiCount(hello var)
{
	int currentRow = ui.regionListSettingsWidget->currentRow();
	int count = var.taxicount().regions_size();
	ui.regionListSettingsWidget->clear();
	for (int i = 0; i < count; i++) {
		QString regionName = QString::fromUtf8(var.taxicount().regions().Get(i).region_name().c_str());
		QString count = QString::number(var.taxicount().regions().Get(i).taxi_count());
		ui.regionListSettingsWidget->addItem(QString("%1 (%2)").arg(regionName).arg(count));
	}
	if (currentRow >= count)
		currentRow = count - 1;
	ui.regionListSettingsWidget->setCurrentRow(currentRow);
	voiceLady->click();
}

void IndigoTaxi::connectedTimerTimeout()
{
	online = true;
	voiceLady->sayPhrase("CONNECTIONOK");
}

void IndigoTaxi::connectionStatus(bool status)
{
	if (status && !online) {
		if (updateStartTimer->isActive()) {
			updateStartTimer->stop();
		} else if (!_updatePerformed) {
			updateStartTimer->start();
		}
		
		connectedTimer->start();
		ui.connectionLabel->setPixmap(QPixmap(":/UI/images/connection-ok.png"));

	} else if (!status && online) {
		updateStartTimer->stop();
		connectedTimer->stop();
		voiceLady->sayPhrase("NOCONNECTION");		
		ui.connectionLabel->setPixmap(QPixmap(":/UI/images/connection-bad.png"));
		online = false;
	}
}

void IndigoTaxi::rebootApp()
{
	// работает плохо
	restartMutex.lock();
	//delete backend;
	QString filePath = QApplication::applicationFilePath();
	QStringList args = QApplication::arguments();
	QString workingDir = QDir::currentPath();
	bool result = QProcess::startDetached(filePath, args, workingDir);

	QApplication::exit(0);
}

// расчёт
void IndigoTaxi::paytimeClick() 
{
	// stop accounting
	if (iTaxiOrder == NULL)
		return;

#ifndef DEBUG
	// здесь какая-то багуля была дважды за день у человека, не пускало заказ дальше
//	if (movementStarted)
//		return;
#endif
		
	iTaxiOrder->stopOrder();
	int payment = iTaxiOrder->calculateSum();

	if (iTaxiOrder->getIsTalon()) {
		ui.finalTalonLabel->setText("ПО ТАЛОНУ");
	} else {
		ui.finalTalonLabel->setText("");
	}

	ui.finalPaymentAmountLabel->setText(QString("%1р.").arg(payment));
	
	// километры
	ui.finalMileageLabel->setText(QString("%1/%2")
		.arg(iTaxiOrder->cityMileage(), 0, 'f', 1)
		.arg(iTaxiOrder->outOfCityMileage(), 0, 'f', 1));
	ui.finalMileageTotalLabel->setText(QString("%1").arg(iTaxiOrder->moneyCity() + iTaxiOrder->moneyMg()));

	ui.finalOverloadLabel->setText(QString("%1/%2")
		.arg(iTaxiOrder->cityMileageOverload(), 0, 'f', 1)
		.arg(iTaxiOrder->outOfCityMileageOverload(), 0, 'f', 1));
	ui.finalOverloadTotalLabel->setText(QString("%1").arg(iTaxiOrder->moneyCityOverload() + iTaxiOrder->moneyMgOverload()));
	
	ui.finalStopsTimeLabel->setText(QString("%1")
		.arg(iTaxiOrder->minutesClientStops()));
	ui.finalTotalTimeTravelledLabel->setText(QString("%1").arg(iTaxiOrder->minutesTotal()));
	ui.finalStopsTotalLabel->setText(QString("%1").arg(iTaxiOrder->minutesClientStops() * iTaxiOrder->orderTaxiRate().client_stop()));

	voiceLady->speakMoney(payment);
	ui.stackedWidget->setCurrentWidget(ui.paytimePage3);
	
	voiceLady->sayPhrase("BYE");
}

// свободен -- сумма оплачивается
void IndigoTaxi::freeButtonClick()
{
	for (int i = 0; i < taxiRegionList.regions_size(); i++) {
		if (iTaxiOrder->getRegionId() == taxiRegionList.regions().Get(i).region_id())
			ui.currentRegionLabel->setText(QString::fromUtf8(taxiRegionList.regions().Get(i).region_name().c_str()));
	}
	if (iTaxiOrder != NULL) {
		backend->sendOrderEvent(hello_TaxiEvent_END_CLIENT_MOVE, iTaxiOrder);
		saveOrderHistory(iTaxiOrder, ITaxiOrder::SUCCESS);
		destroyCurrentOrder();
	}

	ui.trainCrossButton->setChecked(false);
	enableWidget(ui.trainCrossButton, true);
		
	ui.overloadButton->setChecked(false);
	enableWidget(ui.overloadButton, true);

	ui.serverMessage->setPlainText("");
	
	ui.stackedWidget->setCurrentWidget(ui.settingsPage4);
	ui.settingsTabWidget->setCurrentWidget(ui.regionsSettingsTab4);
}

// продолжаем поездку
void IndigoTaxi::resumeVoyageClick()
{
	iTaxiOrder->startOrder();
	ui.stackedWidget->setCurrentWidget(ui.orderPage2);
}

// очистить
void IndigoTaxi::clearMessageClick()
{	
	//QSound::play("click.wav");
	//QSound::play(qApp->applicationDirPath() + QDir::separator() + "stop.wav");
	//if (iTaxiOrder != NULL) {
	//	destroyCurrentOrder();
	//	iTaxiOrder = NULL;
	//}
	
	ui.serverMessage->setPlainText("");
}

void IndigoTaxi::exitButtonClick()
{
	if (confirmDialog->ask("ВЫ ПОДТВЕРЖДАЕТЕ ВЫХОД ИЗ ПРОГРАММЫ? ЗАВЕРШАЕТЕ ЛИ ВЫ СМЕНУ?")) {
		if (driverNumberDialog->showPassword()) {
			qApp->quit();	
		} else {
			infoDialog->info("ОШИБКА: НЕВЕРНЫЙ ПАРОЛЬ");
		}
	}
}

// вернуться
void IndigoTaxi::backToStandByClick()
{
	ui.stackedWidget->setCurrentWidget(ui.standByPage1);
}

void IndigoTaxi::enableWidget(QWidget *widget, bool enable)
{
	widget->setEnabled(enable);
	ui.centralWidget->style()->unpolish(widget);
	ui.centralWidget->style()->polish(widget);
	ui.centralWidget->update();
}

void IndigoTaxi::enableMainButtons(bool enable)
{
	enableWidget(ui.moveToClientButton, enable);
	enableWidget(ui.startClientMoveButton, enable);
}

// клик на все кнопки
void IndigoTaxi::playClick()
{
	voiceLady->click();
}

void IndigoTaxi::enableDutyUI(bool enable) 
{
	if (enable) {
		enableWidget(ui.startClientMoveButton, true);
		ui.dutyStart->setText("КОНЕЦ СМЕНЫ");		
		ui.dutyStart->setProperty("pressed", true);
	} else {
		enableWidget(ui.startClientMoveButton, false);
		ui.dutyStart->setText("НАЧАЛО СМЕНЫ");
		ui.dutyStart->setProperty("pressed", false);
	}
}

/*!
 * \brief
 * На смену, со смены
 * 
 * \param pressed
 * Нажали -- на смену, отжали -- со смены.
 * 
 * Write detailed description for dutyButtonClicked here.
 * 
 * \remarks
 * Write remarks for dutyButtonClicked here.
 * 
 * \see
 * Separate items with the '|' character.
 */
void IndigoTaxi::dutyButtonClicked(bool pressed)
{
	if (!ui.dutyStart->property("pressed").toBool()) {
		if (confirmDialog->ask("Вы подтверждаете начало смены?")) {
			backend->sendEvent(hello_TaxiEvent_ARRIVED);
			enableDutyUI(true);
			infoDialog->info("Смена начата! Выберите район");
			ui.settingsTabWidget->setCurrentWidget(ui.regionsSettingsTab4);
		}
		
	} else {
		if (confirmDialog->ask("Вы подтверждаете конец смены?")) {
			backend->sendEvent(hello_TaxiEvent_DAY_END);
			enableDutyUI(false);
			infoDialog->info("Смена окончена!");
		}
	}
	
	ui.dutyStart->style()->unpolish(ui.dutyStart);
	ui.dutyStart->style()->polish(ui.dutyStart);
	ui.dutyStart->update();
}

void IndigoTaxi::notPayClicked()
{
	if (confirmDialog->ask("ВЫ ПОДТВЕРЖДАЕТЕ, ЧТО КЛИЕНТ НЕ ОПЛАТИЛ ПОЕЗДКУ?")) {
		backend->sendOrderEvent(hello_TaxiEvent_NOT_PAY, lastTaxiOrder);
		infoDialog->info("ДИСПЕТЧЕРУ ОТПРАВЛЕНО УВЕДОМЛЕНИЕ О НЕОПЛАТЕ ПОЕЗДКИ");
	}
}

void IndigoTaxi::dinnerStartClicked()
{
	backend->sendEvent(hello_TaxiEvent_MAY_GO_DINNER);
	showInfoDialog("Диспетчеру отправлен вопрос о вашем уходе на обед");	
}

void IndigoTaxi::dinnerHandleAnswer(hello var) {
	if (var.event() == hello_TaxiEvent_YES_GO_DINNER) {
				
		voiceLady->sayPhrase("MESSAGERECEIVED");
		if (confirmDialog->ask("Диспетчер отпускает Вас на обед. Подтвердите свой уход")) {
			backend->sendEvent(hello_TaxiEvent_GO_DINNER);
			setSettingsStatus("DINNER");
			ui.stackedWidget->setCurrentWidget(ui.settingsPage4);
			ui.settingsTabWidget->setCurrentWidget(ui.driverCabinetSettingsTab2);
			ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageDinner2);
		}
	
	} else if (var.event() == hello_TaxiEvent_NO_GO_DINNER) {		
		voiceLady->sayPhrase("MESSAGERECEIVED");
		showInfoDialog("Диспетчер оставляет вас на линии");
	}
}

void IndigoTaxi::dinnerStopClicked()
{
	backend->sendEvent(hello_TaxiEvent_BACK_DINNER);
	setSettingsStatus("OK");
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPage1);
}


void IndigoTaxi::driverNameChanged(int driverName)
{
	ui.driverNumberButton->setText("ПОЗЫВНОЙ\n" + QString::number(driverName));
	ui.orderPageDriverNumberButton->setText("ПОЗЫВНОЙ\n" + QString::number(driverName));
	driverNumberDialog->setDriverName(QString::number(driverName));
//	ui.driverNameLineEdit->setText(QString::number(driverName));
	settingsIniFile->beginGroup("main");
	settingsIniFile->setValue("driverName", QVariant(driverName));
	settingsIniFile->endGroup();
	settingsIniFile->sync();
}

void IndigoTaxi::driverNameEdited(QString newValue)
{
	backend->setDriverName(newValue.toInt());
}

void IndigoTaxi::awayButtonClicked()
{
	backend->sendEvent(hello_TaxiEvent_MOVE_OUT);
	setSettingsStatus("AWAY");
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageAway4);
}

void IndigoTaxi::awayEndButtonClicked()
{
	backend->sendEvent(hello_TaxiEvent_BACK_MOVE_OUT);
	setSettingsStatus("OK");
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPage1);
}

void IndigoTaxi::fromcarButtonClicked()
{
	backend->sendEvent(hello_TaxiEvent_GO_FROM_CAR);
	setSettingsStatus("FROMCAR");
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageFromcar3);
}

void IndigoTaxi::fromcarEndButtonClicked()
{
	backend->sendEvent(hello_TaxiEvent_BACK_TO_CAR);
	setSettingsStatus("OK");
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPage1);
}

void IndigoTaxi::emptyTripClicked() 
{
	// НЕУСТОЙКА
	if (confirmDialog->ask("ВЫ ПОДТВЕРЖДАЕТЕ НЕУСТОЙКУ ПО ЗАКАЗУ?")) {
		backend->sendOrderEvent(hello_TaxiEvent_EMPTY_TRIP, iTaxiOrder);
		ui.stackedWidget->setCurrentWidget(ui.standByPage1);
		saveOrderHistory(iTaxiOrder, ITaxiOrder::EMPTY_TRIP);
		// сбрасываем заказ
		destroyCurrentOrder();
		clearMessageClick();
		infoDialog->info("ДИСПЕТЧЕРУ ОТПРАВЛЕНО УВЕДОМЛЕНИЕ О НЕУСТОЙКЕ");
	}
}

void IndigoTaxi::repairClicked()
{
	backend->sendEvent(hello_TaxiEvent_GET_DAMAGE);
	setSettingsStatus("REPAIR");
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageRepair5);
}

void IndigoTaxi::repairEndClicked()
{
	backend->sendEvent(hello_TaxiEvent_REPEAR_DAMAGE);
	setSettingsStatus("OK");
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPage1);
}

void IndigoTaxi::techhelpClicked()
{
	backend->sendEvent(hello_TaxiEvent_TECHHELP);
	setSettingsStatus("TECHHELP");
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageTechhelp6);
}

void IndigoTaxi::techhelpBackClicked()
{
	backend->sendEvent(hello_TaxiEvent_BACK_TECHHELP);
	// показываем выбор стоянки
	_changeRegionStopEvent = hello_TaxiEvent_BACK_TECHHELP;
	setSettingsStatus("OK");
	ui.settingsTabWidget->setCurrentWidget(ui.regionsSettingsTab4);
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPage1);
}

/*!
 * \brief
 * Отказ от заказа
 * 
 * \throws <exception class>
 * Description of criteria for throwing this exception.
 * 
 * Write detailed description for notToMeButtonClicked here.
 * 
 * \remarks
 * Write remarks for notToMeButtonClicked here.
 * 
 * \see
 * Separate items with the '|' character.
 */
void IndigoTaxi::notToMeButtonClicked()
{
	if (confirmDialog->ask("ВЫ ПОДТВЕРЖДАЕТЕ ОТКАЗ ОТ ТЕКУЩЕГО ЗАКАЗА?")) {
		orderReceiveTimer->stop();
		backend->sendOrderEvent(hello_TaxiEvent_NOT_TO_ME, iTaxiOrder);
		saveOrderHistory(iTaxiOrder, ITaxiOrder::NOT_TO_ME);
		destroyCurrentOrder();
		ui.serverMessage->setPlainText("");
		infoDialog->info("ДИСПЕТЧЕРУ ОТПРАВЛЕНО УВЕДОМЛЕНИЕ ОБ ОТКАЗЕ ОТ ЗАКАЗА");
	}
}

// реакция на поехали
void IndigoTaxi::selectRegionClicked() 
{
#ifndef DEBUG
	if (!newDirection && satellitesUsed < 5) {
		voiceLady->sayPhrase("NOGPS");
		infoDialog->info("Невозможно начать поездку. Число спутников должно быть больше 5");
		return;
	}
#endif
	ui.stackedWidget->setCurrentWidget(ui.regionListPage6);
}

// какой сейчас тариф
TaxiRatePeriod IndigoTaxi::getCurrentTaxiRatePeriod() {
	int i = 0;
	QTime currentTime = QTime::currentTime();
	int hour = currentTime.hour();
	int minute = currentTime.minute();
	int minutes = hour * 60 + minute;
	for (i = 0; i < taxiRates.rates_size(); i++) {
		TaxiRatePeriod period = taxiRates.rates().Get(i);
		int begin_minutes = period.begin_hour() * 60 + period.begin_minute();
		int end_minutes = period.end_hour() * 60 + period.end_minute();

		if (minutes >= begin_minutes && minutes < end_minutes) 
			return period;
	}

	qDebug() << "rate period not found!!!";
	return TaxiRatePeriod::default_instance();
}

// новый заказ, с номером или без
ITaxiOrder *IndigoTaxi::createTaxiOrder(int order_id, QString address) 
{
	if (iTaxiOrder != NULL) {
		destroyCurrentOrder();
	}
	
	iTaxiOrder = new ITaxiOrder(order_id, getCurrentTaxiRatePeriod(), 
		currentParkingCost, currentParkingId, this);

	iTaxiOrder->setMg(taxiRates.mg());

	connect(backend, SIGNAL(newPosition(QGeoCoordinate)), iTaxiOrder, SLOT(newPosition(QGeoCoordinate)));
	connect(iTaxiOrder, SIGNAL(newMileage(float)), SLOT(newMileage(float)));
	connect(iTaxiOrder, SIGNAL(paymentChanged(int)), SLOT(newPaymentCalculated(int)));

	connect(iTaxiOrder, SIGNAL(newTimeTotal(int)), SLOT(newTimeTotal(int)));
	connect(iTaxiOrder, SIGNAL(newTimeStops(int)), SLOT(newTimeStops(int)));
	connect(iTaxiOrder, SIGNAL(newTimeMovement(int)), SLOT(newTimeMovement(int)));
	connect(iTaxiOrder, SIGNAL(newTimeClientStops(int)), SLOT(newTimeClientStops(int)));

	connect(iTaxiOrder, SIGNAL(movementStartFiltered(bool)), SLOT(movementStartFiltered(bool)));

	connect(this, SIGNAL(orderMovementStart(int)), iTaxiOrder, SLOT(movementStart(int)));

	iTaxiOrder->recalcSum();
	iTaxiOrder->setAddress(address);

	enableWidget(ui.moveToClientButton, true);
	enableWidget(ui.inPlaceButton, true);

	return iTaxiOrder;
}

void IndigoTaxi::destroyCurrentOrder()
{
	if (iTaxiOrder == NULL)
		return;

	orderReceiveTimer->stop();

	disconnect(iTaxiOrder, 0, 0, 0);
	disconnect(backend, 0, iTaxiOrder, 0);
	//delete iTaxiOrder;
	if (lastTaxiOrder != NULL)
	{
		delete lastTaxiOrder;
		lastTaxiOrder = NULL;
	}

	lastTaxiOrder = iTaxiOrder;
	iTaxiOrder = NULL;

	clearMessageClick();

	enableWidget(ui.moveToClientButton, false);
	enableWidget(ui.inPlaceButton, false);

	ui.stackedWidget->setCurrentWidget(ui.standByPage1);
}

void IndigoTaxi::orderReceiveTimerTimeout()
{
	voiceLady->alarm();
	if (orderReceiveCounter > 0) {
		orderReceiveCounter--;
	} else {
		voiceLady->sayPhrase("ORDERABORT");
		backend->sendOrderEvent(hello_TaxiEvent_NOT_ANSWER, iTaxiOrder);
		saveOrderHistory(iTaxiOrder, ITaxiOrder::ABORT_TIMEOUT);
		destroyCurrentOrder();
		clearMessageClick();
		orderReceiveTimer->stop();
		infoDialog->info("ЗАКАЗ НА АДРЕС " + iTaxiOrder->address() +  " ОТМЕНЁН ПО ПРИЧИНЕ ОТСУТСТВИЯ ОТВЕТА ВОДИТЕЛЯ НА ЗАПРОС ДИСПЕТЧЕРА");		
	}
}

// пришёл заказ с сервера, надо реагировать
void IndigoTaxi::handleNewOrder(TaxiOrder taxiOrder)
{
	qDebug() << "Order ID:" << taxiOrder.order_id();
	// заказ с места
	if (iTaxiOrder != NULL && iTaxiOrder->getOrderId() == NO_ORDER_ID) {
		iTaxiOrder->setOrderId(taxiOrder.order_id());
		// что-то ещё отправить
	} else if (iTaxiOrder != NULL) {
		return;
	} else {
		//destroyCurrentOrder();

		iTaxiOrder = createTaxiOrder(taxiOrder.order_id(), QString::fromUtf8(taxiOrder.address().c_str()));

		// талон
		if (taxiOrder.is_talon()) {
			iTaxiOrder->setOrderTaxiRate(taxiOrder.talon_rate());
			iTaxiOrder->setIsTalon(true);
		}

		voiceLady->alarm();

		// ждём столько-то времени
		orderReceiveCounter = 2; // 3 по 10 секунд 
		orderReceiveTimer->setInterval(10 * 1000);
		orderReceiveTimer->start();

		if (taxiOrder.has_address()) {
			ui.serverMessage->setPlainText(QString::fromUtf8(taxiOrder.address().c_str()));
		}
	}
}

void IndigoTaxi::newPaymentCalculated(int payment)
{
	ui.paymentAmountLabel->setText(QString("%1 руб.").arg(payment));
}

void IndigoTaxi::newSpeed(int speed_kmh)
{
	qDebug() << "newSpeed" << speed_kmh;
	ui.speedValueLabel->setText(QString("%1 км/ч").arg(speed_kmh));
}

void IndigoTaxi::newMileage(float mileage)
{
	qDebug() << "newMileage" << mileage;
	ui.distanceValueLabel->setText(QString("%1 км").arg(mileage, 0, 'f', 1));
}
void IndigoTaxi::newSatellitesUsed(int _satellitesUsed)
{
	satellitesUsed = _satellitesUsed;
	ui.gpsSatelliteCountLabel->setText(QString::number(satellitesUsed));
}

void IndigoTaxi::newTimeMovement(int _seconds)
{
}

void IndigoTaxi::newTimeStops(int _seconds)
{
	int hours = _seconds / 3600;
	int minutes = (_seconds % 3600) / 60;
	int seconds = _seconds % 60;	
	
	//ui.timeStopsLabel->setText(QString("%1:%2:%3")
	//	.arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
}

void IndigoTaxi::newTimeClientStops(int _seconds)
{
	int hours = _seconds / 3600;
	int minutes = (_seconds % 3600) / 60;
	int seconds = _seconds % 60;	
	
	ui.timeClientStopsLabel->setText(QString("%1:%2:%3")
		.arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
	
}

void IndigoTaxi::newTimeTotal(int _seconds)
{
	int hours = _seconds / 3600;
	int minutes = (_seconds % 3600) / 60;
	int seconds = _seconds % 60;	
	
	ui.timeTotalLabel->setText(QString("%1:%2:%3")
		.arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));

}

void IndigoTaxi::clientNotExit()
{
	if (confirmDialog->ask("ВЫ ПОДТВЕРЖДАЕТЕ, ЧТО КЛИЕНТ НЕ ВЫШЕЛ?")) {
		backend->sendOrderEvent(hello_TaxiEvent_NOT_EXIT, iTaxiOrder);
		// FIXME ну не вышел, что дальше? Диспетчер по идее названивает дальше клиенту и отменяет заказ кто-то из них уже
		destroyCurrentOrder();
		clearMessageClick();
		infoDialog->info("ДИСПЕТЧЕРУ ОТПРАВЛЕНО УВЕДОМЛЕНИЕ, ЧТО КЛИЕНТ НЕ ВЫШЕЛ. ЗАКАЗ ОТМЕНЁН!");

	}
}

void IndigoTaxi::clientStopClicked(bool on)
{
	if (iTaxiOrder == NULL)
		return;

	if (!movementStarted) {
		iTaxiOrder->setClientStop(on);

		if (on)
		{
			// вдруг это время будет как-то задерживаться уже клиентом?
			//if (!iTaxiOrder->isStarted()) {
			//	iTaxiOrder->startOrder();
			//}
			//voiceLady->sayPhrase("CLIENTSTOP");
			//ui.clientStopButton->setEnabled(false);
		}
	}
}

// 30 секунд отфильтровано в заказе
void IndigoTaxi::movementStartFiltered(bool started)
{
	if (!started && !_stop_sound_played) {
		//voiceLady->sayPhrase("STOP");
		_stop_sound_played = true;
		_start_sound_played = false;
	}
}

void IndigoTaxi::movementStart(int start)
{
	qDebug() << (start ? "movement START" : "movement STOP");

	emit orderMovementStart(start);
	movementStarted = start == 1;

	if (iTaxiOrder == NULL) {
		return;
	}

	if (movementStarted) {
		if (!_start_sound_played) {
			//voiceLady->sayPhrase("ORDERGO");
			_start_sound_played = true;
			_stop_sound_played = false;
		}
		
		iTaxiOrder->setClientStop(false);

		// выключаем переезд
		if (iTaxiOrder->isTrainCross()) {
			iTaxiOrder->setTrainCross(false);

			voiceLady->sayPhrase("TRAINCROSSOFF");

			ui.trainCrossButton->setChecked(false);
			enableWidget(ui.trainCrossButton, true);
		}
	} else { // if (movementStarted)
		iTaxiOrder->setClientStop(true);
	}
}


// нажат переезд, отжимается он сам
void IndigoTaxi::trainCrossButtonClicked()
{
	//if (!movementStarted) {
		voiceLady->sayPhrase("TRAINCROSS");
		ui.trainCrossButton->setEnabled(false);
		if (iTaxiOrder != NULL)
		{
			iTaxiOrder->setTrainCross(true);
		}
	//}
}

void IndigoTaxi::overloadButtonClicked(bool on)
{
	if (on) {
		voiceLady->sayPhrase("OVERLOAD");
	} else {
		voiceLady->sayPhrase("OVERLOADOFF");
	}

	if (iTaxiOrder == NULL)
		return;

	iTaxiOrder->setOverload(on);
	// fixme some messages, maybe
}

void IndigoTaxi::newDirectionClicked()
{
	newDirection = true;

	selectRegionClicked();
}

void IndigoTaxi::cancelRegionSelectClicked()
{
	// если выбирали новый район следования, возвращаемся к заказу. Иначе, к режиму ожидания
	if (!newDirection) {
		ui.stackedWidget->setCurrentWidget(ui.standByPage1);
	} else {
		ui.stackedWidget->setCurrentWidget(ui.orderPage2);
		
	}

	newDirection = false;
	changeRegion = false;
}

void IndigoTaxi::rebootSystem()
{
	// reboot
	if (confirmDialog->ask("ПЕРЕЗАГРУЗИТЬ СИСТЕМУ?")) {
#ifdef UNDER_CE
		SetSystemPowerState(NULL, POWER_STATE_RESET, 0);
#endif
	}
}

QString IndigoTaxi::getSettingsStatus()
{
	settingsIniFile->beginGroup("main");
	QString status = settingsIniFile->value("status", QVariant("OK")).toString();
	settingsIniFile->endGroup();

	return status;
}

void IndigoTaxi::setSettingsStatus(QString status)
{
	settingsIniFile->beginGroup("main");
	settingsIniFile->setValue("status", QVariant(status));
	settingsIniFile->endGroup();
	settingsIniFile->sync();
}

void IndigoTaxi::cancelRegionUpdate()
{
	
}

void IndigoTaxi::showRegionDetailsClicked()
{
	hello var;
	TaxiRegionInfo *info = var.mutable_taxiregioninfo();
	asked_region_id = taxiRegionList.regions().Get(ui.regionListSettingsWidget->currentRow()).region_id();
	var.set_event(hello_TaxiEvent_ASK_REGION_2);
	info->set_region_id(asked_region_id);

	backend->send_message(var);
	
	progressDialog->setModal(true);
	progressDialog->setValue(1);
	progressDialog->show();
}

void IndigoTaxi::backToRegionsSettings()
{
	ui.regionsSettingsStackedWidget->setCurrentWidget(ui.regionsSettingsPage1);
}

void IndigoTaxi::changeDriverRegionStopClicked()
{
	int row = ui.regionDetailsList->currentRow();
	int stop_id = 0;
	if (row < regions_stops_ids.count()) {
		stop_id = regions_stops_ids[row];
	} else {
		infoDialog->info("ОШИБКА. ПОПРОБУЙТЕ СНОВА");		
		ui.regionsSettingsStackedWidget->setCurrentWidget(ui.regionsSettingsPage1);
		return;
	}
	
	QString stop_name;
	stop_name = regions_stops_names[row];

	if (confirmDialog->ask("ВЫ ДЕЙСТВИТЕЛЬНО ХОТИТЕ ПЕРЕЙТИ НА СТОЯНКУ " + stop_name + "?")) {
		hello var;
		TaxiRegionInfo *info = var.mutable_taxiregioninfo();
		info->set_region_id(asked_region_id);
		info->set_stop_id(stop_id);
		if (_changeRegionStopEvent == hello_TaxiEvent_NOTHING) {
			var.set_event(hello_TaxiEvent_CHANGE_REGION);
		} else {
			var.set_event(_changeRegionStopEvent);
			_changeRegionStopEvent = hello_TaxiEvent_NOTHING;
		}

		backend->send_message(var);
		ui.currentRegionLabel->setText(stop_name);

		infoDialog->info("УВЕДОМЛЕНИЕ ДИСПЕТЧЕРУ ОТПРАВЛЕНО");

		ui.regionsSettingsStackedWidget->setCurrentWidget(ui.regionsSettingsPage1);
	}
}

void IndigoTaxi::emergencyButtonClicked()
{
	voiceLady->click();
	backend->sendEvent(hello_TaxiEvent_HELP);
}

void IndigoTaxi::showOrderHistoryClicked()
{
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPageOrders7);
}

void IndigoTaxi::addOrderHistory(QString address, QString status, int sum)
{
	int rc = ui.orderHistoryTable->rowCount();
	ui.orderHistoryTable->setRowCount(rc + 1);
	ui.orderHistoryTable->setItem(rc, 0, new QTableWidgetItem(address));
	ui.orderHistoryTable->setItem(rc, 1, new QTableWidgetItem(status));
	ui.orderHistoryTable->setItem(rc, 2, new QTableWidgetItem(QString::number(sum)));
}

void IndigoTaxi::saveOrderHistory(ITaxiOrder *order, int status)
{
	QString address = order->address();
	if (address == "")
		address = "Без адреса";
	
	switch (status) {
		case ITaxiOrder::EMPTY_TRIP:
			addOrderHistory(address, "Неустойка", 0);
			break;
		case ITaxiOrder::ABORT_DISPATCHER:
			addOrderHistory(address, "Отмена диспетчером", 0);
			break;
		case ITaxiOrder::SUCCESS:
			addOrderHistory(address, "Завершён", order->calculateSum());
			break;
		case ITaxiOrder::ABORT_TIMEOUT:
			addOrderHistory(address, "Нет реакции водителя", 0);
			break;
		case ITaxiOrder::NOT_EXIT:
			addOrderHistory(address, "Клиент не вышел, неустойка", order->getCarIn());
			break;
		case ITaxiOrder::NOT_PAY:
			addOrderHistory(address, "Клиент не оплатил поездку", 0);
			break;
		case ITaxiOrder::NOT_TO_ME:
			addOrderHistory(address, "Вы отказались от заказа", 0);
			break;
		case ITaxiOrder::NO_STATUS:
			addOrderHistory(address, "", 0);
			break;
		default:
			addOrderHistory(address, "", 0);
			break;
	}
}

void IndigoTaxi::backToDriverCabinetSettingsClicked()
{
	ui.driverCabinetSettingsStackWidget->setCurrentWidget(ui.driverCabinetPage1);
}

void IndigoTaxi::taxiRateShowButtonClicked()
{
	ui.systemSettingsStackedWidget->setCurrentWidget(ui.systemSettingsPage2);
}


void IndigoTaxi::taxiRateReturnButtonClicked() 
{
	ui.systemSettingsStackedWidget->setCurrentWidget(ui.systemSettingsPage1);
}

void IndigoTaxi::privateClientButtonClicked()
{
	if (confirmDialog->ask("ВЫ ПОДТВЕРЖДАЕТЕ ПЕРСОНАЛЬНЫЙ ЗАКАЗ?")) {
		destroyCurrentOrder();
		clearMessageClick();
		iTaxiOrder = createTaxiOrder(NO_ORDER_ID, "");
		backend->sendOrderEvent(hello_TaxiEvent_PERSONAL, iTaxiOrder);
	}
}

void IndigoTaxi::infoClicked()
{
	infoDialog->info("ПРОГРАММА IndigoTaxi, версия " + QString(version) + ", 2014 год. Разработчик: ООО \"Системы Индиго\"");
}

void IndigoTaxi::updatesDownloadTipVersionString()
{
	QString versionUrl = "http://indigosystem.ru/taxi-version.txt";
	//QString versionUrl = "http://megaservis.org/taxi-version.txt";
	QUrl version(versionUrl);
	downloader = new FileDownloader(version, this);
	connect(downloader, SIGNAL(downloaded()), SLOT(updatesCheckVersionString()));	

	updateStartTimer->stop();
}

void IndigoTaxi::updateDownloadTimeout()
{
	
	voiceLady->click();
	voiceLady->click();
	voiceLady->click();
	
	downloadManager->pause();
	downloadManager->resume();
}

void IndigoTaxi::downloadProgress(int percent)
{
	qDebug() << "downloaded" << percent << "%";
	
	// timeout reset
	updateDownloadTimeoutTimer->start();
	
	ui.updateInProgressPercentage->setText(QString("%1%").arg(percent));
}

void IndigoTaxi::updatesCheckVersionString()
{
	QByteArray data = downloader->downloadedData();
	QString newVersionString = data.data();
	newVersionString = newVersionString.trimmed();

	QString oldVersionString = version;
	qDebug() << "new version:" << newVersionString << "old version:" << oldVersionString << "updating:" << (newVersionString.trimmed() > oldVersionString.trimmed());
	_updatePerformed = true;
	downloader->deleteLater();
	
	if (newVersionString.trimmed() > oldVersionString.trimmed()) {
		QString newVersionUrlPath = "http://indigosystem.ru/IndigoTaxi.exe.qz";
		//QString newVersionUrlPath = "http://megaservis.org/IndigoTaxi.exe.qz";
		ui.updateInProgressPercentage->setText(QString("%1%").arg(0));
		QUrl url(newVersionUrlPath);
		
		connect(downloadManager, SIGNAL(downloadComplete()), SLOT(newVersionDownloaded()));
		connect(downloadManager, SIGNAL(progress(int)), SLOT(downloadProgress(int)));

		downloadManager->download(url);
		
		ui.updateInProgressIcon->setPixmap(QPixmap(":/UI/images/updateInProgress.png"));
		
		updateDownloadTimeoutTimer->start();
		updateStartTime = QTime::currentTime();
		updateStartTime.start();
		//ui.versionLabel->setText(newVersionString.trimmed());
	}
}

void IndigoTaxi::updateDownloadError(QString reason)
{
	voiceLady->click();
	if (downloader != NULL) {
		downloader->abort();
		downloader->deleteLater();
		downloader = NULL;
	};
	// retry
	updatesDownloadTipVersionString();

	//infoDialog->info("Ошибка установки обновления: " + reason);
}

void IndigoTaxi::newVersionDownloaded()
{
	// TODO проверить целостность обновления
	QFile downloadedApp("download.complete");
	if (!downloadedApp.open(QIODevice::ReadOnly)) {
		infoDialog->info("Ошибка установки обновления: download.complete");
		return;
	}
	
	QByteArray data = qUncompress(downloadedApp.readAll());
	downloadedApp.close();
	QFile::remove("download.complete");

	QFile currentExePath(QApplication::instance()->applicationFilePath());
	QFile downloadedFilePath(QApplication::instance()->applicationDirPath() + "/new_exe.exe");
	
	updateDownloadTimeoutTimer->stop();
	
	if (downloadedFilePath.open(QIODevice::ReadWrite)) {
		qint64 writtenLen = downloadedFilePath.write(data);
		downloadedFilePath.close();

		// хоть какая-то проверка целостности
		if (writtenLen == data.length()) {			
			voiceLady->click();
			voiceLady->click();
			voiceLady->click();
			voiceLady->click();
			voiceLady->click();

			QString oldFilePath = QApplication::instance()->applicationDirPath() + "/old_exe.exe";
			QFile::remove(oldFilePath);
			bool result = !currentExePath.rename(oldFilePath);	
			result |= !downloadedFilePath.rename(QApplication::instance()->applicationFilePath());

			if (confirmDialog->ask("ЗАГРУЖЕНО ОБНОВЛЕНИЕ ПРОГРАММЫ. ВЫПОЛНИТЬ ОБНОВЛЕНИЕ? (ТРЕБУЕТСЯ ПЕРЕЗАГРУЗКА)")) {
				
				// перезагружаемся, тогда всё точно ок
				rebootSystem();
			}
		}
	}
}

void IndigoTaxi::applyColorTheme()
{
	QString style = "";
#if 0
	static int i = 0;
	switch (i) {
		case 0:
			style = "#regionList,#regionListSettingsWidget,#regionDetailsList,#messageTemplatesList{font:14pt \"Segoe UI\";font-weight:700}#regionList QScrollBar:vertical,#regionListSettingsWidget QScrollBar:vertical,#regionDetailsList QScrollBar:vertical,#messageTemplatesList QScrollBar:vertical{width:80px}*{background:#e4e4e7;color:#000}QPushButton{border:2px solid #1d1d1d;color:#1d1d1d}#statusRowFrame *{background:#bdbec2;color:#000}#statusRowFrame{background:#bdbec2}#taxiRateFrame *{background:#e4e4e7;color:#000}#taxiRateFrame{background:#e4e4e7;border-bottom:1px solid #000;border-top:1px solid #101010}#driverNumberButton,#orderPageDriverNumberButton{border:0;color:#909090}#acceptRegionButton{background:#70a400}#backToStandbyButton,#menuButton,#clearButton{background:#fdfdfd;color:#1d1d1d}#moveToClientButton:pressed,#inPlaceButton:pressed,#startClientMoveButton:pressed,#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed,#resumeVoyageButton:pressed,#farawayButton:pressed,#paytimeButton:pressed,#overloadButton:pressed,#trainCrossButton:pressed,#settingsTabWidget QPushButton:pressed{background:#000}#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed{color:#fdfdfd}QTabBar::tab:selected{background:#5bcaff}QTabBar::tab:!selected{background:#dbddde;border:1px solid #000}#moveToClientButton{background:#5bcaff}#inPlaceButton{background:#4cd964}#startClientMoveButton{background:#ff3b30}#inPlaceButton[enabled=\"false\"],#moveToClientButton[enabled=\"false\"],#startClientMoveButton[enabled=\"false\"]{background:#dbddde;color:#364c4e}#driverNumberLabel{color:#909090}#paytimeButton{background:#ff1300}#overloadButton{background:#8e8e93}#overloadButton:checked{background:#1d1d1d}#trainCrossButton{background:#5bcaff}#trainCrossButton:checked{background:#1d1d1d}#directionValueButton{background:#4cd964}#resumeVoyageButton{background:#ea6b00}#freeButton{background:#fdfdfd;color:#1d1d1d}#freeButton:pressed{background:#1d1d1d;color:#fdfdfd}#settingsTabWidget QPushButton{background:#4cd964}#dutyStart:checked{background:#4c66b0}";
			break;
		case 1:
			style = "#regionList,#regionListSettingsWidget,#regionDetailsList,#messageTemplatesList{font:14pt \"Segoe UI\";font-weight:700}#regionList QScrollBar:vertical,#regionListSettingsWidget QScrollBar:vertical,#regionDetailsList QScrollBar:vertical,#messageTemplatesList QScrollBar:vertical{width:80px}*{background:#e4e4e7;color:#000}QPushButton{border:2px solid #1d1d1d;color:#1d1d1d}#statusRowFrame *{background:#bdbec2;color:#000}#statusRowFrame{background:#bdbec2}#taxiRateFrame *{background:#e6e9e2;color:#000}#taxiRateFrame{background:#e6e9e2;border-bottom:1px solid #000;border-top:1px solid #101010}#driverNumberButton,#orderPageDriverNumberButton{border:0;color:#909090}#acceptRegionButton{background:#70a400}#backToStandbyButton,#menuButton,#clearButton{background:#fdfdfd;color:#1d1d1d}#moveToClientButton:pressed,#inPlaceButton:pressed,#startClientMoveButton:pressed,#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed,#resumeVoyageButton:pressed,#farawayButton:pressed,#paytimeButton:pressed,#overloadButton:pressed,#trainCrossButton:pressed,#settingsTabWidget QPushButton:pressed{background:#000}#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed{color:#fdfdfd}QTabBar::tab:selected{background:#5bcaff}QTabBar::tab:!selected{background:#dbddde;border:1px solid #000}#moveToClientButton{background:#5bcaff}#inPlaceButton{background:#4cd964}#startClientMoveButton{background:#ff3b30}#inPlaceButton[enabled=\"false\"],#moveToClientButton[enabled=\"false\"],#startClientMoveButton[enabled=\"false\"]{background:#dbddde;color:#364c4e}#driverNumberLabel{color:#909090}#paytimeButton{background:#ff1300}#overloadButton{background:#8e8e93}#overloadButton:checked{background:#1d1d1d}#trainCrossButton{background:#5bcaff}#trainCrossButton:checked{background:#1d1d1d}#directionValueButton{background:#4cd964}#resumeVoyageButton{background:#ea6b00}#freeButton{background:#fdfdfd;color:#1d1d1d}#freeButton:pressed{background:#1d1d1d;color:#fdfdfd}#settingsTabWidget QPushButton{background:#4cd964}#dutyStart:checked{background:#4c66b0}";
			break;
		case 2:
			style = "#regionList,#regionListSettingsWidget,#regionDetailsList,#messageTemplatesList{font:14pt \"Segoe UI\";font-weight:700}#regionList QScrollBar:vertical,#regionListSettingsWidget QScrollBar:vertical,#regionDetailsList QScrollBar:vertical,#messageTemplatesList QScrollBar:vertical{width:80px}*{background:#e8e8e2;color:#000}QPushButton{border:2px solid #1d1d1d;color:#1d1d1d}#statusRowFrame *{background:#bdbec2;color:#000}#statusRowFrame{background:#bdbec2}#taxiRateFrame *{background:#e8e8e2;color:#000}#taxiRateFrame{background:#e8e8e2;border-bottom:1px solid #000;border-top:1px solid #101010}#driverNumberButton,#orderPageDriverNumberButton{border:0;color:#909090}#acceptRegionButton{background:#70a400}#backToStandbyButton,#menuButton,#clearButton{background:#fdfdfd;color:#1d1d1d}#moveToClientButton:pressed,#inPlaceButton:pressed,#startClientMoveButton:pressed,#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed,#resumeVoyageButton:pressed,#farawayButton:pressed,#paytimeButton:pressed,#overloadButton:pressed,#trainCrossButton:pressed,#settingsTabWidget QPushButton:pressed{background:#000}#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed{color:#fdfdfd}QTabBar::tab:selected{background:#5bcaff}QTabBar::tab:!selected{background:#dbddde;border:1px solid #000}#moveToClientButton{background:#5bcaff}#inPlaceButton{background:#4cd964}#startClientMoveButton{background:#ff3b30}#inPlaceButton[enabled=\"false\"],#moveToClientButton[enabled=\"false\"],#startClientMoveButton[enabled=\"false\"]{background:#dbddde;color:#364c4e}#driverNumberLabel{color:#909090}#paytimeButton{background:#ff1300}#overloadButton{background:#8e8e93}#overloadButton:checked{background:#1d1d1d}#trainCrossButton{background:#5bcaff}#trainCrossButton:checked{background:#1d1d1d}#directionValueButton{background:#4cd964}#resumeVoyageButton{background:#ea6b00}#freeButton{background:#fdfdfd;color:#1d1d1d}#freeButton:pressed{background:#1d1d1d;color:#fdfdfd}#settingsTabWidget QPushButton{background:#4cd964}#dutyStart:checked{background:#4c66b0}";
			break;
		case 3:
			style = "#regionList,#regionListSettingsWidget,#regionDetailsList,#messageTemplatesList{font:14pt \"Segoe UI\";font-weight:700}#regionList QScrollBar:vertical,#regionListSettingsWidget QScrollBar:vertical,#regionDetailsList QScrollBar:vertical,#messageTemplatesList QScrollBar:vertical{width:80px}*{background:#e8e4e3;color:#000}QPushButton{border:2px solid #1d1d1d;color:#1d1d1d}#statusRowFrame *{background:#bdbec2;color:#000}#statusRowFrame{background:#bdbec2}#taxiRateFrame *{background:#e8e4e3;color:#000}#taxiRateFrame{background:#e8e4e3;border-bottom:1px solid #000;border-top:1px solid #101010}#driverNumberButton,#orderPageDriverNumberButton{border:0;color:#909090}#acceptRegionButton{background:#70a400}#backToStandbyButton,#menuButton,#clearButton{background:#fdfdfd;color:#1d1d1d}#moveToClientButton:pressed,#inPlaceButton:pressed,#startClientMoveButton:pressed,#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed,#resumeVoyageButton:pressed,#farawayButton:pressed,#paytimeButton:pressed,#overloadButton:pressed,#trainCrossButton:pressed,#settingsTabWidget QPushButton:pressed{background:#000}#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed{color:#fdfdfd}QTabBar::tab:selected{background:#5bcaff}QTabBar::tab:!selected{background:#dbddde;border:1px solid #000}#moveToClientButton{background:#5bcaff}#inPlaceButton{background:#4cd964}#startClientMoveButton{background:#ff3b30}#inPlaceButton[enabled=\"false\"],#moveToClientButton[enabled=\"false\"],#startClientMoveButton[enabled=\"false\"]{background:#dbddde;color:#364c4e}#driverNumberLabel{color:#909090}#paytimeButton{background:#ff1300}#overloadButton{background:#8e8e93}#overloadButton:checked{background:#1d1d1d}#trainCrossButton{background:#5bcaff}#trainCrossButton:checked{background:#1d1d1d}#directionValueButton{background:#4cd964}#resumeVoyageButton{background:#ea6b00}#freeButton{background:#fdfdfd;color:#1d1d1d}#freeButton:pressed{background:#1d1d1d;color:#fdfdfd}#settingsTabWidget QPushButton{background:#4cd964}#dutyStart:checked{background:#4c66b0}";
			break;
	}
	i = (i + 1) % 4;
	ui.centralWidget->setStyleSheet(style);
	return;
#endif
	switch (colorTheme) {
		case INDIGO_LIGHT_THEME:
			style = "#statusRowFrame *{font-weight: 700}#serverMessage{font-weight:700}#serverMessageLabel{font-weight:700}#taxiRateFrame *{font-weight:700}#regionList,#regionListSettingsWidget,#regionDetailsList,#messageTemplatesList{font:14pt \"Segoe UI\";font-weight:700}#regionList QScrollBar:vertical,#regionListSettingsWidget QScrollBar:vertical,#regionDetailsList QScrollBar:vertical,#messageTemplatesList QScrollBar:vertical{width:80px}*{background:#fff;color:#2b2b2b}QPushButton{color:#1d1d1d;border:2px solid #1d1d1d}#statusRowFrame *{background:#bdbec2;color:#000}#statusRowFrame{background:#bdbec2}#taxiRateFrame *{background:#fff;color:#2b2b2b}#taxiRateFrame{background:#fff;border-top:1px solid #101010;border-bottom:1px solid #000}#driverNumberButton,#orderPageDriverNumberButton{color:#909090;border:0}#acceptRegionButton{background:#70a400}#backToStandbyButton,#menuButton,#clearButton{background:#fdfdfd;color:#1d1d1d}#moveToClientButton:pressed,#inPlaceButton:pressed,#startClientMoveButton:pressed,#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed,#resumeVoyageButton:pressed,#farawayButton:pressed,#paytimeButton:pressed,#overloadButton:pressed,#trainCrossButton:pressed,#settingsTabWidget QPushButton:pressed{background:#000}#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed{color:#fdfdfd}QTabBar::tab:selected{background:#5bcaff}QTabBar::tab:!selected{background:#dbddde;border:1px solid #000}#moveToClientButton{background:#5bcaff}#inPlaceButton{background:#4cd964}#startClientMoveButton{background:#ff3b30}#inPlaceButton[enabled=\"false\"],#moveToClientButton[enabled=\"false\"],#startClientMoveButton[enabled=\"false\"]{background:#dbddde;color:#364c4e}#driverNumberLabel{color:#909090}#paytimeButton{background:#ff1300}#overloadButton{background:#8e8e93}#overloadButton:checked{background:#1d1d1d}#trainCrossButton{background:#5bcaff}#trainCrossButton:checked{background:#1d1d1d}#directionValueButton{background:#4cd964}#resumeVoyageButton{background:#ea6b00}#freeButton{background:#fdfdfd;color:#1d1d1d}#freeButton:pressed{color:#fdfdfd;background:#1d1d1d}#settingsTabWidget QPushButton{background:#4cd964}#dutyStart:checked{background:#4c66b0}";
			
			ui.centralWidget->setStyleSheet(style);
			break;
		case INDIGO_DARK_THEME:
			style = "#regionList,#regionListSettingsWidget,#regionDetailsList,#messageTemplatesList{font:14pt \"Segoe UI\";font-weight:bold}#regionList QScrollBar:vertical,#regionListSettingsWidget QScrollBar:vertical,#regionDetailsList QScrollBar:vertical,#messageTemplatesList QScrollBar:vertical{width:80px}*{background:#1d1d1d;color:white}QPushButton{color:white}#statusRowFrame *{background:black;color:white}#statusRowFrame{background:black}#taxiRateFrame *{background:black;color:#b0b0b0}#taxiRateFrame{background:black;border-top:1px solid #101010}#driverNumberButton,#orderPageDriverNumberButton{color:#909090}#acceptRegionButton{background:#70a400}#backToStandbyButton,#menuButton,#clearButton{background:#fdfdfd;color:#1d1d1d}#moveToClientButton:pressed,#inPlaceButton:pressed,#startClientMoveButton:pressed,#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed,#resumeVoyageButton:pressed,#farawayButton:pressed,#paytimeButton:pressed,#overloadButton:pressed,#trainCrossButton:pressed,#settingsTabWidget QPushButton:pressed{background:black}#backToStandbyButton:pressed,#acceptRegionButton:pressed,#menuButton:pressed,#clearButton:pressed{color:#fdfdfd}QTabBar::tab:selected{background:#63a400}QTabBar::tab:!selected{background:#034900}#moveToClientButton{background:#ea6b00}#inPlaceButton{background:#4ca2a6}#startClientMoveButton{background:#70a400}#inPlaceButton[enabled=\"false\"],#moveToClientButton[enabled=\"false\"],#startClientMoveButton[enabled=\"false\"]{background:#426063;color:#364c4e}#driverNumberLabel{color:#909090}#paytimeButton{background:#c23c00}#overloadButton{background:#4c66b0}#overloadButton:checked{background:#1d1d1d}#trainCrossButton{background:#275883}#trainCrossButton:checked{background:#1d1d1d}#directionValueButton{background:#6c0144}#resumeVoyageButton{background:#ea6b00}#freeButton{background:#fdfdfd;color:#1d1d1d}#freeButton:pressed{color:#fdfdfd;background:#1d1d1d}#settingsTabWidget QPushButton{background:#4ca2a6}#dutyStart:checked{background:#4c66b0}";
			ui.centralWidget->setStyleSheet(style);
			break;
		default:
			break;
	}
}

void IndigoTaxi::switchColorsClicked()
{
	switch (colorTheme) {
		case INDIGO_LIGHT_THEME:
			colorTheme = INDIGO_DARK_THEME;
			break;
		case INDIGO_DARK_THEME:
			colorTheme = INDIGO_LIGHT_THEME;
			break;
		default:
			break;
	}
	applyColorTheme();
}
