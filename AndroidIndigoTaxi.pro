#-------------------------------------------------
#
# Project created by QtCreator 2014-12-06T21:18:35
#
#-------------------------------------------------

QT       += core gui network multimedia positioning

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AndroidIndigoTaxi
TEMPLATE = app

INCLUDEPATH += c:/MinGW/msys/1.0/home/Yury/protobuf-2.5.0/include

win32-g++ {
    QT += serialport
    LIBS += -LC:/Projects/libs -lprotobuf-lite-8 -lwinmm
}

android-g++ {
    DEFINES += UNDER_ANDROID
    LIBS += -lprotobuf-2.5.0
    QT += androidextras
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
    ./resource.h \
    ./settingsform.h \
    ./taxiorder.h \
    ./voicelady.h \
    ./soundeffect.h \
    ./jnisoundeffect.h \
    ./qtsoundeffect.h
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
    ./settingsform.cpp \
    ./taxiorder.cpp \
    ./voicelady.cpp \
    ./soundeffect.cpp \
    ./jnisoundeffect.cpp \
    ./qtsoundeffect.cpp
FORMS += ./drivernumberdialog.ui \
    ./iconfirmationdialog.ui \
    ./iinfodialog.ui \
    ./indigotaxi.ui \
    ./settingsform.ui

RESOURCES += Sound.qrc \
    UI.qrc \
    AndroidSound.qrc

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml\
    android/src/ru/indigosystem/taxi/android/MyQtActivity.java \
    android/src/ru/indigosystem/taxi/android/AudioClient.java \
    android/res/values/libs.xml \
    android/build.gradle

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/android/libs/armeabi-v7a/libprotobuf-2.5.0.so \
        $$PWD/android/libs/armeabi-v7a/libgnustl_shared.so
}

DISTFILES += \
    android/src/ru/indigosystem/taxi/android/HeadsetReceiverRunnable.java \
    android/src/ru/indigosystem/taxi/android/HeadsetReceiver.java \
    android/src/ru/indigosystem/taxi/android/NativeFunctions.java
