#include <QtCore>
#include <QDebug>

#include "isoundplayer.h"
#ifndef UNDER_ANDROID
#include "windows.h"
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
//	PlaySoundW((LPCWSTR)QResource(url).data(), 0, SND_MEMORY | SND_SYNC);
}

void ISoundPlayer::playFileSystemSound(QString filename)
{
//	PlaySoundW((LPCWSTR)filename.utf16(), 0, SND_FILENAME | SND_SYNC);
}
