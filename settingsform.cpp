#include "settingsform.h"

SettingsForm::SettingsForm(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	

}

SettingsForm::~SettingsForm()
{

}

void SettingsForm::setBackend(Backend *backend)
{
	//this->backend = backend;
}

void SettingsForm::newDriverName(QString name)
{
	backend->setDriverName(name.toInt());
}

void SettingsForm::dutyStart()
{
	backend->sendEvent(hello_TaxiEvent_ARRIVED);
}

void SettingsForm::dutyStop()
{
	backend->sendEvent(hello_TaxiEvent_DAY_END);
}

void SettingsForm::dinnerStart()
{
	backend->sendEvent(hello_TaxiEvent_GO_DINNER);
}

void SettingsForm::dinnerStop()
{
	backend->sendEvent(hello_TaxiEvent_BACK_DINNER);
}