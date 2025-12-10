#ifndef PLAYERTANK_H
#define PLAYERTANK_H

#include "bullet.h"

class PlayerTank : public Tank
{
    Q_OBJECT
public:
    PlayerTank(const QPointF&, unsigned short, unsigned short, float);

    void handleKeyPress(Qt::Key);
    void handleKeyRelease(Qt::Key);

    void update(float) override;
    void render(QPainter*) override;
    Bullet* shoot() override;

private:
    float shootCooldown = 3.0f;
    float lastShotTime = 0.0f;
    bool isShooting = false;

    Direction currentDirection = UP;
    bool isMoving = false;

signals:
    void bulletFired(Bullet*);
};

#endif // PLAYERTANK_H
