#ifndef PLAYERTANK_H
#define PLAYERTANK_H

#include "bullet.h"

class PlayerTank : public Tank
{
public:
    PlayerTank(const QPointF&, unsigned short, unsigned short, float);

    void handleKeyPress(Qt::Key);
    void handleKeyRelease(Qt::Key);

    void update(float) override;
    void render(QPainter* painter) override;
    Bullet* shoot() override;

private:
    float shootCooldown = 0.5f;
    float lastShotTime = 0.0f;
    bool shootRequested = false;

    Direction currentDirection = UP;
    bool isMoving = false;

signals:
    void bulletFired(Bullet*);
};

#endif // PLAYERTANK_H
