#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QDialog>
#include "ui_confirmdialog.h"

class confirmdialog : public QDialog
{
	Q_OBJECT

public:
	confirmdialog(QWidget *parent = 0);
	~confirmdialog();

private:
	Ui::confirmdialog ui;
};

#endif // CONFIRMDIALOG_H
