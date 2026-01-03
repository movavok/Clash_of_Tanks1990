#ifndef GAME_H
#define GAME_H

#include <QList>
#include <QDir>
#include <QPointF>
#include <QCoreApplication>
#include <QRandomGenerator>
#include "../entities/playertank.h"
#include "../entities/enemyTypes/enemysniper.h"
#include "../entities/enemyTypes/enemylaser.h"
#include "../entities/bullet.h"
#include "../entities/powerup.h"
#include "../entities/deathmark.h"
#include "../level/level.h"
#include "../systems/audio.h"

class Game : public QObject
{
    Q_OBJECT
public:
    Game();

    void update(float, const QSize&);
    bool checkCollision(Entity*);
    bool checkWindowBounds(Entity*, const QSize&);
    void checkIfShotDown();
    void render(QPainter*);

    void handleKeyPress(Qt::Key);
    void handleKeyRelease(Qt::Key);
    void setMovementScheme(int, Qt::Key);

    void addEntity(Entity*);
    void removeEntity(Entity*);

    void setPaused(bool);
    bool isPaused() const;

    int getMaxLevel() const;
    void advance();
    void finishBox();

    void resetPlayerControls();

    void newGame();
    void restart();

    float calculateLaserLength(const QPointF&, Tank::Direction, float) const;

private:
    PlayerTank* player;
    QList<Entity*> entities;
    Level* level = nullptr;
    int levelIndex = 1;
    int maxLevel;

    int movementScheme = 0;
    Qt::Key shootKey = Qt::Key_Space;

    bool advancing = false;
    bool announcedNoEnemies = false;
    bool paused = false;

    float powerUpSpawnTimer = 0.0;
    float powerUpSpawnInterval = 8.0;

    void initLevel();
    QPointF tileCenter(int, int) const;

    void spawnPlayerAtTile(int, int);
    void spawnEnemiesDefault();

    void updateEntities(float, const QSize&);
    void cleanupDeadEntities();
    bool handlePlayerDeath();
    void handleLevelClear();

    bool enemySeesPlayer(const EnemyTank*) const;

    void detectMaxLevel();
    bool loadLevel(int);
    void advanceLevel();
    void restartLevel();

    void spawnPowerUpRandom();
    void applyPowerUp(PowerUp*);
    void handleTankHit(Tank*);

private slots:
    void onFireLaser(LaserRay* ray);

signals:
    void levelChanged(int);
    void doPlayerDeathBox();
    void doLevelChoiceBox(int);
};

#endif // GAME_H
