#include "drivernumberdialog.h"

DriverNumberDialog::DriverNumberDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QValidator *validator = new QIntValidator(0, 500, this);
	ui.driverNumberLineEdit->setValidator(validator); //add a int validator min value 0 max value 500. This will force the numpad to show, you can also use a QDoubleValidator

	QValidator *validator_2 = new QIntValidator(0, 999999, this);
	ui.passwordEdit->setValidator(validator_2); //add a int validator min value 0 max value 500. This will force the numpad to show, you can also use a QDoubleValidator

}

DriverNumberDialog::~DriverNumberDialog()
{

}

void DriverNumberDialog::setDriverName(QString driverName)
{
	ui.driverNumberLineEdit->setText(driverName);
}

bool DriverNumberDialog::showPassword() 
{
	return exec() == QDialog::Accepted;
}

void DriverNumberDialog::checkPassword()
{
	if (ui.driverNumberLineEdit->text() != "" && ui.passwordEdit->text() == "987321") {		
		_driverNumber = ui.driverNumberLineEdit->text().toInt();
		ui.passwordEdit->setText("");
		accept();
	} else {
		ui.passwordEdit->setText("");
		reject();
	}
}