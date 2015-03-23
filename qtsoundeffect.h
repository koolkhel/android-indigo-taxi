#pragma once

#include "soundeffect.h"
#include <QSoundEffect>

class QtSoundEffect : public SoundEffect
{
    Q_OBJECT
public:
    QtSoundEffect();
    virtual ~QtSoundEffect();

signals:
    void playingChange();
public slots:
    void playSound(QString url);
    bool isPlaying();

private:
    QSoundEffect * player;
    void checker();

};
