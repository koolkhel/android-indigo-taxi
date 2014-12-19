#ifndef ICONFIRMATIONDIALOG_H
#define ICONFIRMATIONDIALOG_H

#include <QDialog>
#include "ui_iconfirmationdialog.h"

class IConfirmationDialog : public QDialog
{
	Q_OBJECT

public:
	IConfirmationDialog(QWidget *parent = 0);
	~IConfirmationDialog();

	void setText(QString text);
	bool ask(QString text);
	bool askYesNo(QString text);
public slots:
	void clickSound();
private:
	Ui::IConfirmationDialog ui;
};

#endif // ICONFIRMATIONDIALOG_H
