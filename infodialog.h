#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include "ui_infodialog.h"

class infodialog : public QDialog
{
	Q_OBJECT

public:
	infodialog(QWidget *parent = 0);
	~infodialog();

private:
	Ui::infodialog ui;
};

#endif // INFODIALOG_H
