#ifndef ISOUNDPLAYER_H
#define ISOUNDPLAYER_H

#include <QObject>

class ISoundPlayer : public QObject
{
	Q_OBJECT

public:
	ISoundPlayer(QObject *parent = 0);
	~ISoundPlayer();
public slots:
	void playFileSystemSound(QString);
	void playResourceSound(QString);

private:
	
};

#endif // ISOUNDPLAYER_H
