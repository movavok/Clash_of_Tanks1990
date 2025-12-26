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
    void useWasdKeys();
    void useArrowKeys();
    void setKeyShoot(Qt::Key);

    void update(float) override;
    void render(QPainter*) override;
    Bullet* shoot() override;

    void resetControls();
    
    void applySpeedBoost(float, float);
    void applyReloadBoost(float);
    void addShield();
    bool hasShield() const;
    void consumeShield();
    void clearAllBuffs();
    void setShootKey(Qt::Key);

private:
    float shootCooldown = 3.0f;
    float lastShotTime = 0.0f;
    bool isShooting = false;

    Qt::Key keyUp = Qt::Key_W, keyDown = Qt::Key_S, keyLeft = Qt::Key_A, keyRight = Qt::Key_D, keyShoot = Qt::Key_Space;
    Direction currentDirection = Direction::UP;
    bool isMoving = false;
    
    float baseSpeed = 0.0f;
    float speedBoostTime = 0.0f;
    float speedBoostDuration = 0.0f;
    float speedMultiplier = 1.0f;
    float reloadBoostTime = 0.0f;
    float reloadBoostDuration = 0.0f;
    int shieldCharges = 0;

    void drawShieldAura(QPainter*) const;
    QPoint drawRotatedSprite(QPainter*, const QPixmap&, QPixmap&) const;
    void drawSpeedTrail(QPainter*, const QPoint&, const QPixmap&) const;
    void drawCooldownBar(QPainter*) const;
    void drawBoostBar(QPainter*, int& barY, float time, float duration, const QColor& color) const;

signals:
    void bulletFired(Bullet*);
};

#endif // PLAYERTANK_H
