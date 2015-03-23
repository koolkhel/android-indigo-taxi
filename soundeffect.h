#ifndef SOUNDEFFECT_H
#define SOUNDEFFECT_H

#include <QObject>
#include <QString>

class SoundEffect : public QObject
{
    Q_OBJECT
public:
    explicit SoundEffect(QObject *parent = 0);
    ~SoundEffect();

signals:
    void playingChanged();
public slots:
    virtual void playSound(QString url);
    bool isPlaying();
protected:
    void mktempFile(QString url);
    void rmtempFile();
    QString temp_file;
};

#endif // SOUNDEFFECT_H
