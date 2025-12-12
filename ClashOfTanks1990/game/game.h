#ifndef GAME_H
#define GAME_H

#include <QList>
#include <QPainter>
#include "../entities/playertank.h"
#include "../entities/enemytank.h"
#include "../entities/bullet.h"
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

private:
    PlayerTank* player;
    QList<Entity*> entities;
    Level* level = nullptr;
};

#endif // GAME_H
