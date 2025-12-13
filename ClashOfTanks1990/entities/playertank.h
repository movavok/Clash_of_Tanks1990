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

    void resetControls();
    
    // Power-up effects
    void applySpeedBoost(float durationSeconds, float multiplier);
    void applyReloadBoost(float durationSeconds);
    void addShield();
    bool hasShield() const;
    void consumeShield();
    void clearAllBuffs();

private:
    float shootCooldown = 3.0f;
    float lastShotTime = 0.0f;
    bool isShooting = false;

    Direction currentDirection = UP;
    bool isMoving = false;
    
    // Power-up state
    float baseSpeed = 0.0f;
    float speedBoostTime = 0.0f;
    float speedMultiplier = 1.0f;
    float reloadBoostTime = 0.0f;
    int shieldCharges = 0;

signals:
    void bulletFired(Bullet*);
};

#endif // PLAYERTANK_H
