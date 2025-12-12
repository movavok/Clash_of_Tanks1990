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

void PlayerTank::render(QPainter* painter) {
    painter->setBrush(Qt::blue);
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
    painter->setBrush(QColor(80, 200, 120));
    painter->drawRect(barFg);
}
