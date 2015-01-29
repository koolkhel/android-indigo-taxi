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

}

ISoundPlayer::~ISoundPlayer()
{

}

void ISoundPlayer::playResourceSound(QString url)
{
    // FIXME sound
    qDebug() << "playing sound " << url;
#ifndef ANDROID
    PlaySoundW((LPCWSTR)QResource(url).data(), 0, SND_MEMORY | SND_SYNC);
#else
    QSound::play(url);
#if 0
    QSound *sound = new QSound(url, this);
    sound->play();
    sound->deleteLater();
#endif
#endif
}

void ISoundPlayer::playFileSystemSound(QString filename)
{
    qDebug() << "playing sound " << filename;
#ifndef ANDROID
    PlaySoundW((LPCWSTR)filename.utf16(), 0, SND_FILENAME | SND_SYNC);
#else
    QSound::play(filename);
#if 0
    QSound *sound = new QSound(filename, this);
    sound->play();
    sound->deleteLater();
#endif
#endif
}
