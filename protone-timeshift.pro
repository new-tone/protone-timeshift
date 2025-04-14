QT += \
    quick \
    quickcontrols2

CONFIG += c++17

include($$PWD/../bass/Bass.pri)

RESOURCES += \
    icons/icons.qrc \
    qml/qml.qrc

SOURCES += \
    src/iconprovider.cpp \
    src/main.cpp \
    src/timeshiftplayer.cpp

HEADERS += \
    src/iconprovider.h \
    src/timeshiftplayer.h
