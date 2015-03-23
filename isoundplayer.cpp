#include <QtCore>
#include <QDebug>

#include "isoundplayer.h"
#include "jnisoundeffect.h"
#include "qtsoundeffect.h"

ISoundPlayer::ISoundPlayer(QObject *parent)
	: QObject(parent)
{
#ifdef UNDER_ANDROID
    player = new JNISoundEffect();
#else
    player = new QtSoundEffect();
#endif    
}

ISoundPlayer::~ISoundPlayer()
{

}

void ISoundPlayer::move(QThread *to)
{
    thread = to;
    player->moveToThread(to);
    connect(player, SIGNAL(playingChanged()), SLOT(playingChanged()),
            Qt::QueuedConnection);
    connect(this, SIGNAL(playSoundSignal(QString)),
            player, SLOT(playSound(QString)), Qt::QueuedConnection);
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

    emit playSoundSignal(uri.toString());
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
        QTimer::singleShot(100, this, SLOT(playingChanged()));
    }
}
