#ifndef ISOUNDPLAYER_H
#define ISOUNDPLAYER_H

#include <QObject>
#include <QSoundEffect>
#include <QMutex>

class ISoundPlayer : public QObject
{
	Q_OBJECT

public:
	ISoundPlayer(QObject *parent = 0);
	~ISoundPlayer();
public slots:
	void playFileSystemSound(QString);
	void playResourceSound(QString);
private slots:
    void stateChanged(QMediaPlayer::State state);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
private:
    void flushQueue();

    QSoundEffect *player;
    QStringList uris;
    QMutex urisLock;
};

#endif // ISOUNDPLAYER_H
