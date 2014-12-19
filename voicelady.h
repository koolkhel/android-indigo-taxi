#ifndef VOICELADY_H
#define VOICELADY_H

#include <QObject>

class VoiceLady : public QObject
{
	Q_OBJECT
	
public:	

	VoiceLady(QObject *parent = 0);
	~VoiceLady();
public slots:
	void speakMoney(int amount);
	void sayPhrase(QString name);
	void alarm();
	void click();
signals:
	void playSound(QString);
	void playSoundFile(QString);
private:
	
};

#endif // VOICELADY_H
