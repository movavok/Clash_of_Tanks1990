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
    return new Bullet(position, currentDirection, 150.0f, this);
}

void EnemyTank::render(QPainter* painter) {
    painter->setBrush(Qt::red); //in progress..
    painter->drawRect(bounds());
}
