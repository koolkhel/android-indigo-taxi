#include "voicelady.h"
#include <QDebug>
#include <QSound>
#include <QtCore>

VoiceLady::VoiceLady(QObject *parent)
	: QObject(parent)
{

}

void VoiceLady::speakMoney(int amount)
{
	/// 123 == 100 + 20 + 3
	int tmp = amount;
	// единицы, десятки, сотни
	int amounts[6] = {0};
	int max_order = 0;

	// получаем разряды цифр
	while (tmp > 0) {
		int a = tmp % 10;
		amounts[max_order] = a;
		tmp /= 10;
		max_order++;
	}
	
	max_order -= 1;

	sayPhrase("COSTANNOUNCE");

	if (amounts[3] != 0) {
		switch (amounts[3]) {
			case 1:
				emit playSound(QString(":/Sound/1na"));
				emit playSound(QString(":/Sound/1000a"));
				break;
			case 2:
				emit playSound(QString(":/Sound/2e"));
				emit playSound(QString(":/Sound/1000i"));
				break;
			case 3:
				emit playSound(QString(":/Sound/3"));
				emit playSound(QString(":/Sound/1000i"));
				break;
			case 4:
				emit playSound(QString(":/Sound/4"));
				emit playSound(QString(":/Sound/1000i"));
				break;
			case 5:
				emit playSound(QString(":/Sound/5"));
				emit playSound(QString(":/Sound/1000"));
				break;
			case 6:
				emit playSound(QString(":/Sound/6"));
				emit playSound(QString(":/Sound/1000"));
				break;
			case 7:
				emit playSound(QString(":/Sound/7"));
				emit playSound(QString(":/Sound/1000"));
				break;
			case 8:
				emit playSound(QString(":/Sound/8"));
				emit playSound(QString(":/Sound/1000"));
				break;
			case 9:
				emit playSound(QString(":/Sound/9"));
				emit playSound(QString(":/Sound/1000"));
				break;
		}
	}
	if (max_order >= 2) {
		emit playSound(QString(":/Sound/") 
			+ QString::number(amounts[2] * 100));
	}

	if (amounts[1] == 1) { // десятки
		emit playSound(QString(":/Sound/") 
			+ QString::number(10 * amounts[1] + amounts[0]));
	} else if (amounts[1] == 0) { // нули
		if (amounts[0] != 0) {
			emit playSound(QString(":/Sound/") 
				+ QString::number(amounts[0]));
		}
	} else {
		emit playSound(QString(":/Sound/") 
			+ QString::number(amounts[1] * 10));
		if (amounts[0] != 0) {
			emit playSound(QString(":/Sound/") 
				+ QString::number(amounts[0]));
		}
	}

	if (amounts[1] == 1) {
		emit playSound(QString(":/Sound/roubles"));
	} else {
		switch (amounts[0]) {
	case 0:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		emit playSound(QString(":/Sound/roubles"));
		break;		
	case 2:
	case 3:
	case 4:
		emit playSound(QString(":/Sound/roubla"));
		break;
	case 1:
		emit playSound(QString(":/Sound/roubl"));
		break;
		}
	}
}
void VoiceLady::alarm() {
	emit playSoundFile("click.wav");
	emit playSoundFile("click.wav");
	emit playSoundFile("click.wav");
	emit playSoundFile("click.wav");
}

void VoiceLady::click()
{
	emit playSoundFile("click.wav");
}

void VoiceLady::sayPhrase(QString name)
{
	QString resourceName = ":/Sound/" + name;
	// FIXME temporary sound disable
//#ifdef UNDER_CE
	emit playSound(resourceName);	
//#endif
}

VoiceLady::~VoiceLady()
{

}
