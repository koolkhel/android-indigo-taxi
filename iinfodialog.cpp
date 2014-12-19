#include "iinfodialog.h"

#include <QSound>

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
	QSound::play("click.wav");
}