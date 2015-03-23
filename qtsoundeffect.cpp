#include "qtsoundeffect.h"
#include <QDebug>

QtSoundEffect::QtSoundEffect()
{
    player = new QSoundEffect(this);
    connect(player, &QSoundEffect::playingChanged, this, &QtSoundEffect::checker);

}

QtSoundEffect::~QtSoundEffect()
{
    disconnect(player, 0, 0 , 0);
    delete player;
}

void QtSoundEffect::playSound(QString url) {

    if(!isPlaying()) {
        mktempFile(url);
        player->setSource(QUrl::fromLocalFile(temp_file));
        player->play();
    } else {
        player->stop();
    }
}

bool QtSoundEffect::isPlaying()
{
    return player->isPlaying();
}

void QtSoundEffect::checker()
{
    if (!isPlaying() || player->status() == QSoundEffect::Error) {
        rmtempFile();       
    }
    Q_EMIT playingChanged();
}
