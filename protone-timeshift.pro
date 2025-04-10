QT += \
    quick \
    quickcontrols2

CONFIG += c++17

include($$PWD/../bass/Bass.pri)

RESOURCES += \
    qml/qml.qrc

SOURCES += \
    src/main.cpp \
    src/TimeshiftPlayer.cpp

HEADERS += \
    src/TimeshiftPlayer.h
