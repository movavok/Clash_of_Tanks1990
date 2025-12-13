#include "enemytank.h"

EnemyTank::EnemyTank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd)
    : Tank(pos, wth, hgt, spd)
{
    srand(static_cast<unsigned int>(time(nullptr))); //random generator
    baseSpeed = spd;
}

void EnemyTank::update(float deltaTime) {
    // Tick power-up timers
    if (speedBoostTime > 0.0f) {
        speedBoostTime -= deltaTime;
        if (speedBoostTime <= 0.0f) { speedBoostTime = 0.0f; speedMultiplier = 1.0f; }
    }
    if (reloadBoostTime > 0.0f) {
        reloadBoostTime -= deltaTime;
        if (reloadBoostTime <= 0.0f) reloadBoostTime = 0.0f;
    }

    // Apply speed multiplier
    speed = baseSpeed * speedMultiplier;

    if (isMoving) move(currentDirection, deltaTime);
    lastShotTime += deltaTime;

    float effectiveCooldown = reloadBoostTime > 0.0f ? 1.0f : shootCooldown;
    if (lastShotTime >= effectiveCooldown) {
        Bullet* newBullet = shoot();
        if (newBullet) emit bulletFired(newBullet);
    }

    changeTimer += deltaTime;
    if (changeTimer >= 0.5f) {
        if (rand() % 2 == 0) //50%
            currentDirection = static_cast<Direction>(rand() % 4);
        changeTimer = 0.0f;
    }
}

Bullet* EnemyTank::shoot() {
    lastShotTime = 0.0f;
    QPointF bulletPos;

    switch (currentDirection) {
    case UP:
        bulletPos = QPointF(position.x() + width/2 - Bullet::getBulletSize()/2, position.y() - Bullet::getBulletSize()); break;
    case DOWN:
        bulletPos = QPointF(position.x() + width/2 - Bullet::getBulletSize()/2, position.y() + height); break;
    case LEFT:
        bulletPos = QPointF(position.x() - Bullet::getBulletSize(), position.y() + height/2 - Bullet::getBulletSize()/2); break;
    case RIGHT:
        bulletPos = QPointF(position.x() + width, position.y() + height/2 - Bullet::getBulletSize()/2); break;
    }
    return new Bullet(bulletPos, currentDirection, 150.0f, this);
}

void EnemyTank::render(QPainter* painter) {
    // Shield visual: different color when shield active (purple-red)
    QColor tankColor = shieldCharges > 0 ? QColor(200, 60, 200) : QColor(220, 0, 0);
    painter->setBrush(tankColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(bounds());

    // Speed trail visual when boost active (different color from player)
    if (speedBoostTime > 0.0f) {
        QRectF trail;
        const qreal trailLen = 10.0;
        switch (currentDirection) {
        case UP:    trail = QRectF(position.x(), position.y() + height, width, trailLen); break;
        case DOWN:  trail = QRectF(position.x(), position.y() - trailLen, width, trailLen); break;
        case LEFT:  trail = QRectF(position.x() + width, position.y(), trailLen, height); break;
        case RIGHT: trail = QRectF(position.x() - trailLen, position.y(), trailLen, height); break;
        }
        painter->setBrush(QColor(240, 80, 160, 120));
        painter->drawRect(trail);
    }

    // Cooldown bar above the tank
    float denom = (reloadBoostTime > 0.0f ? 1.0f : shootCooldown);
    float percent = lastShotTime / denom;
    if (percent < 0.0f) percent = 0.0f; else if (percent > 1.0f) percent = 1.0f;

    QRectF barBg(position.x(), position.y() - 6.0, width, 4.0);
    painter->setBrush(QColor(60, 60, 60));
    painter->drawRect(barBg);

    QRectF barFg(position.x(), position.y() - 6.0, width * percent, 4.0);
    painter->setBrush(reloadBoostTime > 0.0f ? QColor(255, 100, 180) : QColor(240, 180, 60));
    painter->drawRect(barFg);
}

void EnemyTank::applySpeedBoost(float durationSeconds, float multiplier) {
    speedBoostTime = durationSeconds;
    speedMultiplier = multiplier > 0.0f ? multiplier : 1.0f;
}

void EnemyTank::applyReloadBoost(float durationSeconds) {
    reloadBoostTime = durationSeconds;
}

void EnemyTank::addShield() { if (shieldCharges < 1) shieldCharges = 1; }
bool EnemyTank::hasShield() const { return shieldCharges > 0; }
void EnemyTank::consumeShield() { if (shieldCharges > 0) shieldCharges -= 1; }
void EnemyTank::clearAllBuffs() {
    speedBoostTime = 0.0f;
    speedMultiplier = 1.0f;
    reloadBoostTime = 0.0f;
    shieldCharges = 0;
    speed = baseSpeed;
}
