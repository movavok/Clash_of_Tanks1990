#ifndef GAME_H
#define GAME_H

#include <QList>
#include <QPainter>
#include "../entities/entity.h"
#include "../entities/playertank.h"
#include "../entities/enemytank.h"

class Game
{
public:
    Game();

    void update(float);
    bool checkCollision(Entity*);
    void render(QPainter*);

    void handleKeyPress(Qt::Key);
    void handleKeyRelease(Qt::Key);

    void addEntity(Entity*);
    void removeEntity(Entity*);

private:
    PlayerTank* player;
    QList<Entity*> entities;
};

#endif // GAME_H
