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
    urisLock.lock();

    if (uris.empty()) {
        qDebug() << "empty sound list";
        urisLock.unlock();

        return;
    }

    qDebug() << "flushQueue player state" << player->isPlaying();
    if (player->isPlaying()) {
        urisLock.unlock();

        return;
    }

    QUrl uri = QUrl(uris.at(0));
    uris.removeFirst();
    urisLock.unlock();

    qDebug() << "flushQueue setMedia " << uri;
    player->setSource(uri);
    player->play();
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
    } else if (!uris.empty()){
        QTimer::singleShot(50, this, SLOT(playingChanged()));
    }
}
