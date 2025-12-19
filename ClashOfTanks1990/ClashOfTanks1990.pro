QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    entities/bullet.cpp \
    entities/enemytank.cpp \
    entities/entity.cpp \
    entities/playertank.cpp \
    entities/powerup.cpp \
    entities/deathmark.cpp \
    entities/tank.cpp \
    game/game.cpp \
    game/gameview.cpp \
    level/level.cpp \
    \
    main.cpp \
    systems/audio.cpp \
    ui\mainwindow.cpp \
    

HEADERS += \
    entities/bullet.h \
    entities/enemytank.h \
    entities/entity.h \
    entities/playertank.h \
    entities/powerup.h \
    entities/deathmark.h \
    entities/tank.h \
    game/game.h \
    game/gameview.h \
    level/level.h \
    \
    systems/audio.h \
    ui\mainwindow.h \
    

FORMS += \
    ui\mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_FILE = app_icon.rc

RESOURCES += \
    resources\resources.qrc

