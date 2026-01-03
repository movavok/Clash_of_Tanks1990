#ifndef ENEMYLASER_H
#define ENEMYLASER_H

#include "../enemytank.h"

class EnemyLaser : public EnemyTank
{
    Q_OBJECT
public:
    EnemyLaser(const QPointF&, PlayerTank*, const QList<Entity*>*);
    void update(float) override;

protected:
    void tryShoot(float) override;
    IndicatorType currentIndicator() const override;

private:
    bool isShootingLaser = false;
    float laserDuration = 3.0f;
    float laserTimer = 0.0f;
    LaserRay* shootLaser();
};

#endif // ENEMYLASER_H
