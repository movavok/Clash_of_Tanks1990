#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include "playertank.h"

class EnemyTank : public Tank
{
    Q_OBJECT
public:
    EnemyTank(const QPointF&, unsigned short, unsigned short, float, PlayerTank*);

    void update(float) override;
    void render(QPainter*) override;
    Bullet* shoot() override;

    void setTileSize(int);
    void setSeesPlayer(bool);

    void applySpeedBoost(float, float);
    void applyReloadBoost(float);
    void addShield();
    bool hasShield() const;
    void consumeShield();
    void clearAllBuffs();

private:
    PlayerTank* player = nullptr;

    float shootCooldown = 1.5f;
    float lastShotTime = 0.0f;

    enum class BehaviorState { Patrol, Chase };
    BehaviorState state = BehaviorState::Patrol;
    void decideBehavior(float);
    void patrolBehavior(float);
    void chaseBehavior(float);

    float behaviorTimer = 0.0f;
    float reactionTimer = 0.0f;

    Direction currentDirection = Direction::DOWN;
    bool isMoving = true;

    int tileSize = 0;

    float baseSpeed = 0.0f;
    float speedBoostTime = 0.0f;
    float speedBoostDuration = 0.0f;
    float speedMultiplier = 1.0f;
    float reloadBoostTime = 0.0f;
    float reloadBoostDuration = 0.0f;
    int shieldCharges = 0;

    void updateBoosts(float);

    Direction turnToPlayer() const;
    void tryShoot(float);

    bool seesPlayer = false;
    bool canSeePlayer() const;

    void drawShieldAura(QPainter*) const;
    QPoint drawRotatedSprite(QPainter*, const QPixmap&, QPixmap&) const;
    void drawSpeedTrail(QPainter*, const QPoint&, const QPixmap&) const;
    void drawCooldownBar(QPainter*) const;
    void drawBoostBar(QPainter*, int&, float, float, const QColor&) const;

signals:
    void bulletFired(Bullet*);
};

#endif // ENEMYTANK_H
