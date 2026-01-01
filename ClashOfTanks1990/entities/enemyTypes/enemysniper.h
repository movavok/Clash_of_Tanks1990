#ifndef ENEMYSNIPER_H
#define ENEMYSNIPER_H

#include "../enemytank.h"

class EnemySniper : public EnemyTank
{
    Q_OBJECT
public:
    EnemySniper(const QPointF&, PlayerTank*, const QList<Entity*>*);
    void update(float) override;

protected:
    bool canShoot() const override;
    IndicatorType currentIndicator() const override;

private:
    bool canSeePlayer() const override;
    bool aimedAtPlayer() const;
};

#endif // ENEMYSNIPER_H
