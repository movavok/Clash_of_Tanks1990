#include "playertank.h"

PlayerTank::PlayerTank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd)
    : Tank(pos, wth, hgt, spd) {}

void PlayerTank::handleKeyPress(Qt::Key key) {
    switch(key) {
    case Qt::Key_W: currentDirection = UP;    isMoving = true; break;
    case Qt::Key_S: currentDirection = DOWN;  isMoving = true; break;
    case Qt::Key_A: currentDirection = LEFT;  isMoving = true; break;
    case Qt::Key_D: currentDirection = RIGHT; isMoving = true; break;
    case Qt::Key_Space: isShooting = true; break;
    default: break;
    }
}

void PlayerTank::handleKeyRelease(Qt::Key key) {
    if ((key == Qt::Key_W && currentDirection == UP) ||
        (key == Qt::Key_S && currentDirection == DOWN) ||
        (key == Qt::Key_A && currentDirection == LEFT) ||
        (key == Qt::Key_D && currentDirection == RIGHT))
        isMoving = false;
    if (key == Qt::Key_Space) isShooting = false;
}

void PlayerTank::update(float deltaTime) {
    if (isMoving) move(currentDirection, deltaTime);
    lastShotTime += deltaTime;

    if (isShooting && lastShotTime >= shootCooldown) {
        Bullet* newBullet = shoot();
        if (newBullet) emit bulletFired(newBullet);
    }
}

Bullet* PlayerTank::shoot() {
    lastShotTime = 0.0f;
    return new Bullet(position, currentDirection, 150.0f, this);
}

void PlayerTank::render(QPainter* painter) {
    painter->setBrush(Qt::blue); //in progress..
    painter->drawRect(bounds());
}
