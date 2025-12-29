#ifndef ENEMYTANK_H
#define ENEMYTANK_H

#include "playertank.h"

class EnemyTank : public Tank
{
    Q_OBJECT
public:
    EnemyTank(const QPointF&, unsigned short, unsigned short, float, PlayerTank*);

    virtual void update(float) override;
    virtual void render(QPainter*) override;

    void setTileSize(int);
    void setSeesPlayer(bool);

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

    float behaviorTimer = 0.0f;
    float reactionTimer = 0.0f;

    Bullet::BulletType bulletType = Bullet::BulletType::Default;
    bool canShowEye = true;

    Direction currentDirection = Direction::DOWN;
    bool isMoving = true;
    bool seesPlayer = false;

    virtual Bullet* shoot() override;

    Direction turnToPlayer() const;
    virtual bool canShoot() const;
    void tryShoot(float);

private:
    float lastShotTime = 0.0f;

    enum class BehaviorState { Patrol, Chase };
    BehaviorState state = BehaviorState::Patrol;
    void decideBehavior(float);
    void patrolBehavior(float);
    void chaseBehavior(float);

    int tileSize = 0;

    float baseSpeed = 0.0f;
    float speedBoostTime = 0.0f;
    float speedBoostDuration = 0.0f;
    float speedMultiplier = 1.0f;
    float reloadBoostTime = 0.0f;
    float reloadBoostDuration = 0.0f;
    bool reloadBoostWasActive = false;
    int shieldCharges = 0;

    void updateBoosts(float);

    virtual bool canSeePlayer() const;

    void drawShieldAura(QPainter*) const;
    QPoint drawRotatedSprite(QPainter*, const QPixmap&, QPixmap&) const;
    void drawSpeedTrail(QPainter*, const QPoint&, const QPixmap&) const;
    void drawCooldownBar(QPainter*) const;
    void drawBoostBar(QPainter*, int&, float, float, const QColor&) const;

signals:
    void bulletFired(Bullet*);
};

#endif // ENEMYTANK_H
