#include "enemylaser.h"

EnemyLaser::EnemyLaser(const QPointF& pos, PlayerTank* player, const QList<Entity*>* list)
    : EnemyTank(pos, 30, 30, 100.0f, player, list)
{
    shootCooldown = 6.0f;
    viewRange = 6;
    spritePath = ":/tanks/enemyLaser.png";
    shootSoundID = "shootLaser";
    bulletType = Bullet::BulletType::None;
}

void EnemyLaser::update(float dt) {
    if (isShootingLaser) {
        laserTimer += dt;
        isMoving = false;

        lastShotTime += dt;
        if (laserTimer >= laserDuration) {
            isShootingLaser = false;
            laserTimer = 0.0f;
            isMoving = true;
        }
        return;
    } else {
        EnemyTank::update(dt);
        tryShoot(dt);
    }
}

void EnemyLaser::tryShoot(float dt) {
    if (isShootingLaser) return;
    lastShotTime += dt;

    float cooldown = reloadBoostTime > 0.0f ? 1.0f : shootCooldown;
    if (lastShotTime < cooldown) return;
    if (!canShoot()) return;

    LaserRay* ray = shootLaser();
    isShootingLaser = true;
    laserTimer = 0.0f;
    if (ray) emit laserFired(ray);

    Audio::play(shootSoundID);
}

EnemyTank::IndicatorType EnemyLaser::currentIndicator() const {
    if (isShootingLaser) return IndicatorType::Focus;
    return EnemyTank::currentIndicator();
}

LaserRay* EnemyLaser::shootLaser() {
    lastShotTime = 0.0f;
    float sizeCoef = (reloadBoostTime > 0.0f) ? 1.6f : 1.0f;

    LaserRay* ray = new LaserRay(QPointF(0, 0), currentDirection, laserDuration, sizeCoef, this);

    float drawWth = (currentDirection == Direction::LEFT || currentDirection == Direction::RIGHT) ? ray->getSegmentLength() : ray->getThickness();
    float drawHgt = (currentDirection == Direction::LEFT || currentDirection == Direction::RIGHT) ? ray->getThickness() : ray->getSegmentLength();

    QPointF rayPos;
    switch (currentDirection) {
    case Direction::UP:
        rayPos = QPointF(position.x() + width/2 - drawWth/2, position.y() - drawHgt); break;
    case Direction::DOWN:
        rayPos = QPointF(position.x() + width/2 - drawWth/2, position.y() + height); break;
    case Direction::LEFT:
        rayPos = QPointF(position.x() - drawWth, position.y() + height/2 - drawHgt/2); break;
    case Direction::RIGHT:
        rayPos = QPointF(position.x() + width, position.y() + height/2 - drawHgt/2); break;
    }
    ray->setPosition(rayPos);
    return ray;
}


