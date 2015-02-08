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
    player = new QMediaPlayer(this, QMediaPlayer::LowLatency);
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));
}

ISoundPlayer::~ISoundPlayer()
{

}

void ISoundPlayer::flushQueue()
{
    if (uris.empty())
        return;

    qDebug() << "flushQueue player state" << player->state();
    if (player->state() != QMediaPlayer::StoppedState)
        return;

    urisLock.lock();
    QUrl uri = QUrl(uris.at(0));
    qDebug() << "flushQueue setMedia " << uri;
    player->setMedia(uri);
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

void ISoundPlayer::stateChanged(QMediaPlayer::State state)
{
    qDebug() << "player status " << state;
    switch (state) {
    case QMediaPlayer::StoppedState:
        qDebug() << "player stopped";
        break;
    }
}

void ISoundPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "media status " << status;
    switch (status) {
    case QMediaPlayer::LoadedMedia:
        player->play();
        break;
    case QMediaPlayer::EndOfMedia:
        flushQueue();
        break;
    case QMediaPlayer::InvalidMedia:
        qDebug() << "invalid media detected";
        flushQueue();
        break;
    }
}
