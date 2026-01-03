#include "enemysniper.h"

EnemySniper::EnemySniper(const QPointF& pos, PlayerTank* player, const QList<Entity*>* list)
    : EnemyTank(pos, 30, 30, 70.0f, player, list)
{
    bulletSpeed = 450.0f;
    shootCooldown = 4.0f;
    viewRange = 19;
    reactionRange = 60.0f;
    spritePath = ":/tanks/enemySniper.png";
    shootSoundID = "sniperShot";
    bulletType = Bullet::BulletType::Sniper;
}

void EnemySniper::update(float dt) {
    if (canSeePlayer()) {
        if (reactionTimer > 0.6f) {
            currentDirection = turnToPlayer();
            reactionTimer = 0.0f;
        }
        if (aimedAtPlayer()) isMoving = false;
        else isMoving = true;
    }
    EnemyTank::update(dt);
}

bool EnemySniper::canShoot() const { return aimedAtPlayer(); }

EnemySniper::IndicatorType EnemySniper::currentIndicator() const {
    if (aimedAtPlayer()) return IndicatorType::Aim;
    return EnemyTank::currentIndicator();
}

bool EnemySniper::aimedAtPlayer() const {
    QPointF delta = player->getPosition() - position;
    if (!seesPlayer) return false;
    if (player && player->ifHidden()) return false;

    switch (currentDirection) {
    case Direction::UP:    return std::abs(delta.x()) < 10 && delta.y() < 0;
    case Direction::DOWN:  return std::abs(delta.x()) < 10 && delta.y() > 0;
    case Direction::LEFT:  return std::abs(delta.y()) < 10 && delta.x() < 0;
    case Direction::RIGHT: return std::abs(delta.y()) < 10 && delta.x() > 0;
    }
    return false;
}


