#ifndef GAME_H
#define GAME_H

#include <QList>
#include <QPointF>
#include <QCoreApplication>
#include <QMessageBox>
#include <QAbstractButton>
#include <QRandomGenerator>
#include <algorithm>
#include "../entities/playertank.h"
#include "../entities/enemytank.h"
#include "../entities/bullet.h"
#include "../entities/powerup.h"
#include "../entities/deathmark.h"
#include "../level/level.h"

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

    void addEntity(Entity*);
    void removeEntity(Entity*);

    void setPaused(bool);
    bool isPaused() const;
    void restart();

private:
    PlayerTank* player;
    QList<Entity*> entities;
    Level* level = nullptr;
    int levelIndex = 1;
    bool advancing = false;
    bool announcedNoEnemies = false;
    bool paused = false;

    float powerUpSpawnTimer = 0.0;
    float powerUpSpawnInterval = 8.0;

    void initLevel();
    QPointF tileCenter(int, int) const;

    void spawnPlayerAtTile(int, int);
    void spawnEnemiesDefault();

    void doMessage(int);
    void updateEntities(float, const QSize&);
    void cleanupDeadEntities();
    bool handlePlayerDeath();
    void handleLevelClear();

    void advanceLevel();
    void restartLevel();

    void spawnPowerUpRandom();
    void applyPowerUp(PowerUp*);

signals:
    void levelChanged(int);
};

#endif // GAME_H
