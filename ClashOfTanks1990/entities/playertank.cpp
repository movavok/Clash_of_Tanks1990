#include "playertank.h"

PlayerTank::PlayerTank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd)
    : Tank(pos, wth, hgt, spd) {
    baseSpeed = spd;
}

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
    if (speedBoostTime > 0.0f) {
        speedBoostTime -= deltaTime;
        if (speedBoostTime <= 0.0f) {
            speedBoostTime = 0.0f;
            speedMultiplier = 1.0f;
        }
    }
    if (reloadBoostTime > 0.0f) {
        reloadBoostTime -= deltaTime;
        if (reloadBoostTime <= 0.0f) reloadBoostTime = 0.0f;
    }

    speed = baseSpeed * speedMultiplier;

    if (isMoving) move(currentDirection, deltaTime);
    lastShotTime += deltaTime;

    float effectiveCooldown = reloadBoostTime > 0.0f ? 1.0f : shootCooldown;

    if (isShooting && lastShotTime >= effectiveCooldown) {
        Bullet* newBullet = shoot();
        if (newBullet) emit bulletFired(newBullet);
    }
}

Bullet* PlayerTank::shoot() {
    lastShotTime = 0.0f;
    QPointF bulletPos;
    unsigned short currentBulletSize = reloadBoostTime > 0.0f ? 8 : Bullet::getDefaultBulletSize();

    switch (currentDirection) {
    case UP:
        bulletPos = QPointF(position.x() + width/2 - currentBulletSize/2, position.y() - currentBulletSize); break;
    case DOWN:
        bulletPos = QPointF(position.x() + width/2 - currentBulletSize/2, position.y() + height); break;
    case LEFT:
        bulletPos = QPointF(position.x() - currentBulletSize, position.y() + height/2 - currentBulletSize/2); break;
    case RIGHT:
        bulletPos = QPointF(position.x() + width, position.y() + height/2 - currentBulletSize/2); break;
    }
    return new Bullet(bulletPos, currentDirection, 150.0f, this, currentBulletSize);
}

void PlayerTank::render(QPainter* painter) {
    static QPixmap playerSprite("../../assets/tanks/playerTank.png");
    if (!playerSprite.isNull()) {
        int angleDeg = 0;
        switch (currentDirection) {
        case UP:    angleDeg = 0;   break;
        case RIGHT: angleDeg = 90;  break;
        case DOWN:  angleDeg = 180; break;
        case LEFT:  angleDeg = 270; break;
        }

        QTransform rotation;
        rotation.rotate(angleDeg);
        QPixmap rotated = playerSprite.transformed(rotation, Qt::SmoothTransformation);

        QPixmap scaled = rotated.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const int drawX = static_cast<int>(position.x() + (width - scaled.width()) / 2.0f);
        const int drawY = static_cast<int>(position.y() + (height - scaled.height()) / 2.0f);
        painter->drawPixmap(drawX, drawY, scaled);
    } else {
        QColor tankColor = shieldCharges > 0 ? QColor(70, 200, 255) : QColor(0, 0, 255);
        painter->setBrush(tankColor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(bounds());
    }

    // Speed trail
    if (speedBoostTime > 0.0f && isMoving) {
        QRectF trail;
        const qreal trailLen = 10.0;
        switch (currentDirection) {
        case UP: trail = QRectF(position.x(), position.y() + height, width, trailLen); break;
        case DOWN: trail = QRectF(position.x(), position.y() - trailLen, width, trailLen); break;
        case LEFT: trail = QRectF(position.x() + width, position.y(), trailLen, height); break;
        case RIGHT: trail = QRectF(position.x() - trailLen, position.y(), trailLen, height); break;
        }
        painter->setBrush(QColor(80, 160, 255, 120));
        painter->drawRect(trail);
    }

    // Cooldown bar
    float effective = reloadBoostTime > 0.0f ? 1.0f : shootCooldown;
    float denom = effective > 0.0f ? effective : 1.0f;
    float percent = lastShotTime / denom;
    if (percent < 0.0f) percent = 0.0f; else if (percent > 1.0f) percent = 1.0f;

    QRectF barBg(position.x(), position.y() - 6.0, width, 4.0);
    painter->setBrush(QColor(60, 60, 60));
    painter->drawRect(barBg);

    QRectF barFg(position.x(), position.y() - 6.0, width * percent, 4.0);
    painter->setBrush(reloadBoostTime > 0.0f ? QColor(255, 170, 50) : QColor(80, 200, 120));
    painter->drawRect(barFg);
}

void PlayerTank::resetControls() {
    isMoving = false;
    isShooting = false;
}

void PlayerTank::applySpeedBoost(float durationSeconds, float multiplier) {
    speedBoostTime = durationSeconds;
    speedMultiplier = multiplier > 0.0f ? multiplier : 1.0f;
}

void PlayerTank::applyReloadBoost(float durationSeconds) { reloadBoostTime = durationSeconds; }

void PlayerTank::addShield() { if (shieldCharges < 1) shieldCharges = 1; }
bool PlayerTank::hasShield() const { return shieldCharges > 0; }
void PlayerTank::consumeShield() { if (shieldCharges > 0) shieldCharges -= 1; }

void PlayerTank::clearAllBuffs() {
    speedBoostTime = 0.0f;
    speedMultiplier = 1.0f;
    reloadBoostTime = 0.0f;
    shieldCharges = 0;
    speed = baseSpeed;
}
