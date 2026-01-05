#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include "playertank.h"
#include "laserray.h"
#include "powerup.h"

class EnemyTank : public Tank
{
    Q_OBJECT
public:
    EnemyTank(const QPointF&, unsigned short, unsigned short, float, PlayerTank*, const QList<Entity*>*);

    virtual void update(float) override;
    void render(QPainter*) override;

    void setTileSize(int);
    int getTileSize() const;
    void setSeesPlayer(bool);
    void setSeesBoost(bool);
    unsigned short getViewRange() const;

    void applySpeedBoost(float, float);
    void applyReloadBoost(float);
    void addShield();
    bool hasShield() const;
    void consumeShield();
    void clearAllBuffs();

protected:
    PlayerTank* player = nullptr;
    QString spritePath = ":/tanks/enemyTank.png";
    QString shootSoundID = "shoot";
    float bulletSpeed = 150.0f;
    float shootCooldown = 1.5f;
    unsigned short viewRange = 6;

    float chargeTimer = 0.0f;
    float bulletSpeedMult = 1.0f;

    float behaviorTimer = 0.0f;
    float reactionTimer = 0.0f;
    float reactionRange = 20.0f;

    Bullet::BulletType bulletType = Bullet::BulletType::Default;
    enum class IndicatorType { None, Chase, Dodge, Aim, Focus, Gift };
    virtual EnemyTank::IndicatorType currentIndicator() const;
    bool canShowEye = true;

    Direction currentDirection = Direction::DOWN;
    bool isMoving = true;
    bool seesPlayer = false;
    bool seesBoost = false;

    Bullet* shoot() override;

    Direction turnToPlayer() const;
    virtual bool canShoot() const;
    bool canSeePlayer() const;
    virtual void tryShoot(float);

    float lastShotTime = 0.0f;
    float baseSpeed = 0.0f;
    float speedBoostTime = 0.0f;
    float speedBoostDuration = 0.0f;
    float speedMultiplier = 1.0f;
    float reloadBoostTime = 0.0f;
    float reloadBoostDuration = 0.0f;
    bool reloadBoostWasActive = false;
    int shieldCharges = 0;

private:
    enum class BehaviorState { Patrol, Chase, Dodge, Collect };
    BehaviorState state = BehaviorState::Patrol;
    void decideBehavior(float);
    void patrolBehavior(float);
    void chaseBehavior();
    void dodgeBehavior(float);
    void collectBehavior();

    PowerUp* nearestPowerUp(QPointF&) const;
    Direction turnToPoint(const QPointF&) const;

    float dodgeCooldown = 0.0f;
    float dodgeTimer = 0.0f;
    float dodgeSpeedCoef = 1.6f;
    QPointF lastBulletPos;
    QPointF lastBulletDir;
    const QList<Entity*>* entities;

    bool bulletNearby();
    int tileSize = 0;

    void updateBoosts(float);

    void drawShieldAura(QPainter*) const;
    QPoint drawRotatedSprite(QPainter*, const QPixmap&, QPixmap&) const;
    void drawSpeedTrail(QPainter*, const QPoint&, const QPixmap&) const;
    void drawCooldownBar(QPainter*) const;
    void drawBoostBar(QPainter*, int&, float, float, const QColor&) const;

    void checkPrevPosition(float);
    QPointF lastSeenPos;
    float stuckTimer = 0.0f;
    Tank::Direction unstuckDirection() const;


signals:
    void bulletFired(Bullet*);
    void laserFired(LaserRay* ray);
};

#endif // ENEMYTANK_H
