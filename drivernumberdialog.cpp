#include "drivernumberdialog.h"
#include <QDebug>

DriverNumberDialog::DriverNumberDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

    //QValidator *validator = new QIntValidator(0, 500, this);
    //ui.driverNumberLineEdit->setValidator(validator); //add a int validator min value 0 max value 500. This will force the numpad to show, you can also use a QDoubleValidator

    //QValidator *validator_2 = new QIntValidator(0, 999999, this);
    //ui.passwordEdit->setValidator(validator_2); //add a int validator min value 0 max value 500. This will force the numpad to show, you can also use a QDoubleValidator

    ui.driverNumberLineEdit->setAttribute(Qt::WA_InputMethodEnabled, true);
    ui.driverNumberLineEdit->setFocusPolicy(Qt::StrongFocus);
    ui.driverNumberLineEdit->setInputMethodHints(Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText);
    ui.driverNumberLineEdit->setFocus();

    ui.passwordEdit->setAttribute(Qt::WA_InputMethodEnabled, true);
    ui.passwordEdit->setFocusPolicy(Qt::StrongFocus);
    ui.passwordEdit->setText("");
    ui.passwordEdit->setInputMethodHints(Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText);
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
    bool result = false;
//    qApp->inputMethod()->show();
    result = exec() == QDialog::Accepted;
    qApp->inputMethod()->hide();
    return result;
}

void DriverNumberDialog::checkPassword()
{
    qDebug() << "==========================================";
    qDebug() << "xxx" << ui.passwordEdit->text() << "xxx";
    qDebug() << "xxx" << ui.driverNumberLineEdit->text() << "xxx";
    qDebug() << "==========================================";
	if (ui.driverNumberLineEdit->text() != "" && ui.passwordEdit->text() == "987321") {		
		_driverNumber = ui.driverNumberLineEdit->text().toInt();
		ui.passwordEdit->setText("");
		accept();
	} else {
		ui.passwordEdit->setText("");
		reject();
	}
}
