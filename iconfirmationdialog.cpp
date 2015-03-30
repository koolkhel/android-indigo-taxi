#include "iconfirmationdialog.h"

#include "voicelady.h"

IConfirmationDialog::IConfirmationDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

IConfirmationDialog::~IConfirmationDialog()
{

}

void IConfirmationDialog::setText(QString text)
{
	ui.textBrowser->setText(text);	
}

bool IConfirmationDialog::ask(QString text)
{
	ui.pushButton->setText("НАЗАД");
	ui.pushButton_2->setText("ОК");
	ui.textBrowser->setFontPointSize(16);
	setText(text);
    // FIXME
    setGeometry(0, 36, 800, 444);
	return exec() == QDialog::Accepted;
}

bool IConfirmationDialog::askYesNo(QString text)
{
	ui.pushButton->setText("НЕТ");
	ui.pushButton_2->setText("ДА");
	ui.textBrowser->setFontPointSize(14);
	setText(text);
	return exec() == QDialog::Accepted;
}

void IConfirmationDialog::clickSound()
{
    voiceLady->click();
}
