#ifndef ISOUNDPLAYER_H
#define ISOUNDPLAYER_H

#include <QObject>
#include <QSoundEffect>
#include <QMutex>
#include <QThread>

#include "soundeffect.h"

class ISoundPlayer : public QObject
{
	Q_OBJECT

public:
	ISoundPlayer(QObject *parent = 0);
	~ISoundPlayer();
    void move(QThread *to);
signals:
    void playSoundSignal(QString);
public slots:
	void playFileSystemSound(QString);
	void playResourceSound(QString);
private slots:
    void playingChanged();
private:
    void flushQueue();

    SoundEffect *player;
    QThread *thread;
    QStringList uris;
    QMutex urisLock;
};

#endif // ISOUNDPLAYER_H
