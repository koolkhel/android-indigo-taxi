#include "soundeffect.h"
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

SoundEffect::SoundEffect(QObject *parent) : QObject(parent)
{ }

SoundEffect::~SoundEffect()
{
    rmtempFile();
}

void SoundEffect::playSound(QString) {
}

bool SoundEffect::isPlaying() {
    return false;
}

void SoundEffect::mktempFile(QString url)
{
    url.remove("qrc");
    QFile myFile(url);
    if(myFile.open(QIODevice::ReadOnly)) {
        QString file_path = QStandardPaths::standardLocations(
                                QStandardPaths::AppDataLocation
                    )[0];

        QDir dir(file_path);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        url.remove(":/audio/").remove(":/Sound/").replace("/", "_").replace(":", "_");
        temp_file = file_path + "/" + url;

        QFile myFile2(temp_file);
        if (myFile2.open(QIODevice::WriteOnly)) {
            myFile2.write(myFile.readAll());
            qDebug() << "temp_file == " << temp_file;
        } else {
            qDebug() << "cannot write myFile2 " << temp_file;
        }

    } else {
        qDebug() << "cannot use resource " << url;
    }
}

void SoundEffect::rmtempFile()
{
    if(QFile::exists(temp_file)) {
        QFile::remove(temp_file);
    }
}

       
