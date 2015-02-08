#include <QtCore>
#include <QDebug>

#include "isoundplayer.h"
#ifndef UNDER_ANDROID
#include "windows.h"
#else
#include <QSound>
#endif


ISoundPlayer::ISoundPlayer(QObject *parent)
	: QObject(parent)
{
    player = new QSoundEffect(this);
    connect(player, SIGNAL(playingChanged()), SLOT(playingChanged()));
}

ISoundPlayer::~ISoundPlayer()
{

}

void ISoundPlayer::flushQueue()
{
    if (uris.empty())
        return;

    qDebug() << "flushQueue player state" << player->isPlaying();
    if (player->isPlaying())
        return;

    urisLock.lock();
    QUrl uri = QUrl(uris.at(0));
    qDebug() << "flushQueue setMedia " << uri;
    player->setSource(uri);
    player->play();
    uris.removeFirst();
    urisLock.unlock();
}

void ISoundPlayer::playResourceSound(QString url)
{
    // :/Resources/...
    qDebug() << "playing resource sound " << url;

    urisLock.lock();
    uris.append("qrc" + url);
    urisLock.unlock();

    flushQueue();
}

void ISoundPlayer::playFileSystemSound(QString filename)
{
    qDebug() << "playing filesystem sound " << filename;

    urisLock.lock();
    uris.append(QUrl::fromLocalFile(filename).toString());
    urisLock.unlock();

    flushQueue();
}

void ISoundPlayer::playingChanged()
{
    qDebug() << "player status " << player->isPlaying();
    if (!player->isPlaying()) {
        flushQueue();
    }
}
