QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/config.cpp \
    core/timer.cpp \
    entities/bullet.cpp \
    entities/enemytank.cpp \
    entities/entity.cpp \
    entities/playertank.cpp \
    entities/powerup.cpp \
    entities/tank.cpp \
    game/game.cpp \
    game/gameview.cpp \
    level/level.cpp \
    level/levelmanager.cpp \
    main.cpp \
    systems/audio.cpp \
    systems/collision.cpp \
    systems/input.cpp \
    systems/physics.cpp \
    ui\mainwindow.cpp \
    utils/renderer.cpp \
    utils/sprite.cpp

HEADERS += \
    core/GameState.h \
    core/config.h \
    core/timer.h \
    entities/bullet.h \
    entities/enemytank.h \
    entities/entity.h \
    entities/playertank.h \
    entities/powerup.h \
    entities/tank.h \
    game/game.h \
    game/gameview.h \
    level/level.h \
    level/levelmanager.h \
    systems/audio.h \
    systems/collision.h \
    systems/input.h \
    systems/physics.h \
    ui\mainwindow.h \
    utils/renderer.h \
    utils/sprite.h

FORMS += \
    ui\mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources\resources.qrc
