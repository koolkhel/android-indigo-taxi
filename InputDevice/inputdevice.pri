DEPENDPATH +=  InputDevice
INCLUDEPATH += InputDevice

SOURCES += InputDevice/inputdevice.cpp \
        InputDevice/numpad.cpp \
        InputDevice/keyboard.cpp \

HEADERS += InputDevice/inputdevice.h \
        InputDevice/numpad.h \
        InputDevice/keyboard.h \
        InputDevice/customedit.h \


FORMS += InputDevice/numpad.ui \
        InputDevice/keyboard.ui
