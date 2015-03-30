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
    // FIXME
    setGeometry(0, 36, 800, 444);
	exec();
}

void IInfoDialog::clickSound()
{
    voiceLady->click();
}
