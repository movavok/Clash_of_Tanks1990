#include "enemytank.h"

EnemyTank::EnemyTank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd)
    : Tank(pos, wth, hgt, spd)
{
    srand(static_cast<unsigned int>(time(nullptr))); //random generator
}

void EnemyTank::update(float deltaTime) {
    if (isMoving) move(currentDirection, deltaTime);
    lastShotTime += deltaTime;

    if (lastShotTime >= shootCooldown) {
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
    painter->setBrush(Qt::red);
    painter->setPen(Qt::NoPen);
    painter->drawRect(bounds());

    // Cooldown bar above the tank
    float denom = shootCooldown > 0.0f ? shootCooldown : 1.0f;
    float percent = lastShotTime / denom;
    if (percent < 0.0f) percent = 0.0f; else if (percent > 1.0f) percent = 1.0f;

    QRectF barBg(position.x(), position.y() - 6.0, width, 4.0);
    painter->setBrush(QColor(60, 60, 60));
    painter->drawRect(barBg);

    QRectF barFg(position.x(), position.y() - 6.0, width * percent, 4.0);
    painter->setBrush(QColor(240, 180, 60));
    painter->drawRect(barFg);
}
