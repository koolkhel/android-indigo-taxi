#ifndef DRIVERNUMBERDIALOG_H
#define DRIVERNUMBERDIALOG_H

#include <QDialog>
#include "ui_drivernumberdialog.h"

class DriverNumberDialog : public QDialog
{
	Q_OBJECT
public:
    enum Mode {
        DRIVER_NUMBER = 1,
        TAXI_ORG_NUMBER = 2
    };

    DriverNumberDialog(QWidget *parent = 0);
	~DriverNumberDialog();

	int driverNumber() { return _driverNumber; }
signals:
	void driverNumberChanged(int driverNumber);
public slots:
	void checkPassword();
    bool showPassword(int mode = DRIVER_NUMBER);
	void setDriverName(QString driverName);

private:
	Ui::DriverNumberDialog ui;
	int _driverNumber;
};

#endif // DRIVERNUMBERDIALOG_H
