#ifndef DRIVERNUMBERDIALOG_H
#define DRIVERNUMBERDIALOG_H

#include <QDialog>
#include "ui_drivernumberdialog.h"

class DriverNumberDialog : public QDialog
{
	Q_OBJECT

public:
	DriverNumberDialog(QWidget *parent = 0);
	~DriverNumberDialog();

	int driverNumber() { return _driverNumber; }
signals:
	void driverNumberChanged(int driverNumber);
public slots:
	void checkPassword();
	bool showPassword();
	void setDriverName(QString driverName);

private:
	Ui::DriverNumberDialog ui;
	int _driverNumber;
};

#endif // DRIVERNUMBERDIALOG_H
