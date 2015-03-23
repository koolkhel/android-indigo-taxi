#include "iinfodialog.h"

#include "voicelady.h"

IInfoDialog::IInfoDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

IInfoDialog::~IInfoDialog()
{

}

void IInfoDialog::setText(QString text)
{
	ui.textBrowser->setText(text);
}

void IInfoDialog::info(QString text)
{
	setText(text);
	exec();
}

void IInfoDialog::clickSound()
{
    voiceLady->click();
}
