#ifndef ENEMYSNIPER_H
#define ENEMYSNIPER_H

#include "../enemytank.h"

class EnemySniper : public EnemyTank
{
    Q_OBJECT
public:
    EnemySniper(const QPointF&, PlayerTank*);

    void update(float) override;

protected:
    bool canShoot() const override;

private:
    bool canSeePlayer() const override;
    bool aimedAtPlayer() const;
};

#endif // ENEMYSNIPER_H
