#-------------------------------------------------
#
# Project created by QtCreator 2014-12-06T21:18:35
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AndroidIndigoTaxi
TEMPLATE = app

INCLUDEPATH += c:/MinGW/msys/1.0/home/Yury/protobuf-2.5.0/include

win32-g++ {
    QT += serialport
    LIBS += -LC:/Projects/libs -lprotobuf-lite-8
}

android-g++ {
    DEFINES += UNDER_ANDROID
    LIBS += -lprotobuf-2.5.0
}

HEADERS += ./backend.h \
    ./downloadmanager.h \
    ./drivernumberdialog.h \
    ./filedownloader.h \
    ./hello.pb.h \
    ./iconfirmationdialog.h \
    ./iinfodialog.h \
    ./indigoqueue.h \
    ./indigotaxi.h \
    ./isoundplayer.h \
    ./logger.h \
    ./qgeocoordinate.h \
    ./qgeocoordinate_p.h \
    ./qgeopositioninfo.h \
    ./qgeopositioninfosource.h \
    ./qgeopositioninfosourcefactory.h \
    ./qgeosatelliteinfo.h \
    ./qgeosatelliteinfosource.h \
    ./qlocationutils_p.h \
    ./qmobilityglobal.h \
    ./qmobilitypluginsearch.h \
    ./qnmeapositioninfosource.h \
    ./qnmeapositioninfosource_p.h \
    ./resource.h \
    ./settingsform.h \
    ./taxiorder.h \
    ./voicelady.h
SOURCES += ./backend.cpp \
    ./hello.pb.cc \
    ./downloadmanager.cpp \
    ./drivernumberdialog.cpp \
    ./filedownloader.cpp \
    ./iconfirmationdialog.cpp \
    ./iinfodialog.cpp \
    ./indigoqueue.cpp \
    ./indigotaxi.cpp \
    ./isoundplayer.cpp \
    ./logger.cpp \
    ./main.cpp \
    ./qgeocoordinate.cpp \
    ./qgeopositioninfo.cpp \
    ./qgeopositioninfosource.cpp \
    ./qgeopositioninfosourcefactory.cpp \
    ./qgeosatelliteinfo.cpp \
    ./qlocationutils.cpp \
    ./qnmeapositioninfosource.cpp \
    ./settingsform.cpp \
    ./taxiorder.cpp \
    ./voicelady.cpp
FORMS += ./drivernumberdialog.ui \
    ./iconfirmationdialog.ui \
    ./iinfodialog.ui \
    ./indigotaxi.ui \
    ./settingsform.ui

# include(InputDevice/inputdevice.pri)

CONFIG += mobility
MOBILITY = 

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/android/libs/armeabi-v7a/libprotobuf-2.5.0.so \
        $$PWD/android/libs/armeabi-v7a/libgnustl_shared.so
}

