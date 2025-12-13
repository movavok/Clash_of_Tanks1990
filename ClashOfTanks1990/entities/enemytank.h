#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include "bullet.h"
#include <cstdlib>
#include <ctime>

class EnemyTank : public Tank
{
    Q_OBJECT
public:
    EnemyTank(const QPointF&, unsigned short, unsigned short, float);

    void update(float) override;
    void render(QPainter* painter) override;
    Bullet* shoot() override;

private:
    float shootCooldown = 1.5f;
    float lastShotTime = 0.0f;
    float changeTimer = 0.0f;

    Direction currentDirection = DOWN;
    bool isMoving = true;

    // Power-up state (non-stacking)
    float baseSpeed = 0.0f;
    float speedBoostTime = 0.0f;
    float speedMultiplier = 1.0f;
    float reloadBoostTime = 0.0f;
    int shieldCharges = 0;

public:
    // Effects API used by Game
    void applySpeedBoost(float durationSeconds, float multiplier);
    void applyReloadBoost(float durationSeconds);
    void addShield();
    bool hasShield() const;
    void consumeShield();
    void clearAllBuffs();

signals:
    void bulletFired(Bullet*);
};

#endif // ENEMYTANK_H
