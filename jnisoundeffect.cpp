#include "jnisoundeffect.h"

#include <QDebug>
#include <QMutex>

#ifdef __ANDROID__
    #include <QAndroidJniObject>
#endif

JNISoundEffect::JNISoundEffect()
{
    _status = false;
}

JNISoundEffect::~JNISoundEffect()
{
}

void JNISoundEffect::playSound(QString url)
{
    qDebug() << "playSound enter";
    soundMutex.lock();
    if(!_status) {
        _status = true;
        Q_EMIT playingChanged();
        mktempFile(url);
#ifdef __ANDROID__
    QAndroidJniObject string1 = QAndroidJniObject::fromString(temp_file);
    QAndroidJniObject::callStaticMethod<jint>("ru/indigosystem/taxi/android/AudioClient",
                                              "playaudio", "(Ljava/lang/String;)I",
                                              string1.object<jstring>());
#endif
        finalyze();
    }
    qDebug() << "playSound exit";
    soundMutex.unlock();
}

bool JNISoundEffect::isPlaying()
{
    return _status;
}

void JNISoundEffect::finalyze()
{
    rmtempFile();
    _status = false;
    Q_EMIT playingChanged();
}
