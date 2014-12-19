#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QWidget>
#include "ui_settingsform.h"

#include "backend.h"

class SettingsForm : public QWidget
{
	Q_OBJECT

public:
	SettingsForm(QWidget *parent = 0);
	~SettingsForm();

	void setBackend(Backend *backend);
public slots:
	void newDriverName(QString name);

	void dutyStart();
	void dutyStop();
	void dinnerStart();
	void dinnerStop();

private:
	Ui::SettingsForm ui;
	Backend *backend;
};

#endif // SETTINGSFORM_H
