#pragma once

#include "soundeffect.h"

#include <QMutex>

class JNISoundEffect : public SoundEffect
{
    Q_OBJECT
public:
    JNISoundEffect();
    virtual ~JNISoundEffect();

signals:
    void playingChange();
public slots:
    void playSound(QString url);
    bool isPlaying();

private:
    bool _status;
    Q_SLOT void finalyze();

    QMutex soundMutex;
};
