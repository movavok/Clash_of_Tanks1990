#include "enemysniper.h"

EnemySniper::EnemySniper(const QPointF& pos, PlayerTank* player)
    : EnemyTank(pos, 30, 30, 70.0f, player)
{
    bulletSpeed = 450.0f;
    shootCooldown = 4.0f;
    spritePath = ":/tanks/enemySniper.png";
    shootSoundID = "shootLaser";
    bulletType = Bullet::BulletType::Sniper;
}

bool EnemySniper::canSeePlayer() const {
    if (!player || player->ifHidden()) return false;
    if (!seesPlayer) return false;
    return true;
}

void EnemySniper::update(float dt) {
    if (canSeePlayer()) {
        if (reactionTimer > 0.6f) {
            currentDirection = turnToPlayer();
            reactionTimer = 0.0f;
        }
        if (aimedAtPlayer()) {
            isMoving = false;
            tryShoot(dt);
        } else isMoving = true;
    }
    EnemyTank::update(dt);
}

bool EnemySniper::canShoot() const { return aimedAtPlayer(); }

bool EnemySniper::aimedAtPlayer() const {
    QPointF delta = player->getPosition() - position;
    if (!seesPlayer) return false;

    switch (currentDirection) {
    case Direction::UP:    return std::abs(delta.x()) < 10 && delta.y() < 0;
    case Direction::DOWN:  return std::abs(delta.x()) < 10 && delta.y() > 0;
    case Direction::LEFT:  return std::abs(delta.y()) < 10 && delta.x() < 0;
    case Direction::RIGHT: return std::abs(delta.y()) < 10 && delta.x() > 0;
    }
    return false;
}


