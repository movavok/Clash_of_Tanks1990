#include "playertank.h"

PlayerTank::PlayerTank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd)
    : Tank(pos, wth, hgt, spd) { baseSpeed = spd; }

void PlayerTank::handleKeyPress(Qt::Key key) {
    if (key == keyUp) { currentDirection = UP;    isMoving = true; }
    else if (key == keyDown) { currentDirection = DOWN;  isMoving = true; }
    else if (key == keyLeft) { currentDirection = LEFT;  isMoving = true; }
    else if (key == keyRight) { currentDirection = RIGHT; isMoving = true; }
    else if (key == keyShoot) { isShooting = true; }
}

void PlayerTank::handleKeyRelease(Qt::Key key) {
    if ((key == keyUp && currentDirection == UP) ||
        (key == keyDown && currentDirection == DOWN) ||
        (key == keyLeft && currentDirection == LEFT) ||
        (key == keyRight && currentDirection == RIGHT))
        isMoving = false;
    if (key == keyShoot) isShooting = false;
}

void PlayerTank::useWasdKeys() { keyUp = Qt::Key_W; keyDown = Qt::Key_S; keyLeft = Qt::Key_A; keyRight = Qt::Key_D; }
void PlayerTank::useArrowKeys() { keyUp = Qt::Key_Up; keyDown = Qt::Key_Down; keyLeft = Qt::Key_Left; keyRight = Qt::Key_Right; }

void PlayerTank::setKeyShoot(Qt::Key key) { keyShoot = key; }

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
        Audio::play("shoot");
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
    static QPixmap playerSprite(":/tanks/playerTank.png");
    drawShieldAura(painter);
    if (!playerSprite.isNull()) {
        QPixmap scaledSprite;
        QPoint drawPos = drawRotatedSprite(painter, playerSprite, scaledSprite);
        drawSpeedTrail(painter, drawPos, scaledSprite);
    } else {
        QColor tankColor = shieldCharges > 0 ? QColor(70, 200, 255) : QColor(0, 0, 255);
        painter->setBrush(tankColor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(bounds());
    }

    drawCooldownBar(painter);

    // Boost bars
    int barY = static_cast<int>(position.y() - 12.0);
    drawBoostBar(painter, barY, speedBoostTime,  speedBoostDuration,  QColor(60, 190, 255));
    drawBoostBar(painter, barY, reloadBoostTime, reloadBoostDuration, QColor(255, 170, 50));
}

void PlayerTank::drawShieldAura(QPainter* painter) const {
    if (!hasShield()) return;
    painter->save();
    const double shieldMargin = 6.0;
    QRectF shieldRect(position.x() - shieldMargin,
                      position.y() - shieldMargin,
                      width + shieldMargin * 2.0,
                      height + shieldMargin * 2.0);
    painter->setBrush(QColor(70, 200, 255, 110));
    painter->setPen(QPen(QColor(70, 200, 255, 160), 2.0));
    painter->drawEllipse(shieldRect);
    painter->restore();
}

QPoint PlayerTank::drawRotatedSprite(QPainter* painter, const QPixmap& sprite, QPixmap& outScaled) const {
    int angleDeg = 0;
    switch (currentDirection) {
    case UP:    angleDeg = 0;   break;
    case RIGHT: angleDeg = 90;  break;
    case DOWN:  angleDeg = 180; break;
    case LEFT:  angleDeg = 270; break;
    }
    QTransform rotation;
    rotation.rotate(angleDeg);
    QPixmap rotated = sprite.transformed(rotation, Qt::SmoothTransformation);
    outScaled = rotated.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const int drawX = static_cast<int>(position.x() + (width - outScaled.width()) / 2.0f);
    const int drawY = static_cast<int>(position.y() + (height - outScaled.height()) / 2.0f);
    painter->drawPixmap(drawX, drawY, outScaled);
    return QPoint(drawX, drawY);
}

void PlayerTank::drawSpeedTrail(QPainter* painter, const QPoint& mainDrawPos, const QPixmap& scaledSprite) const {
    if (!(speedBoostTime > 0.0f) || !isMoving) return;
    const int offset1 = 10;
    const int offset2 = 20;
    int dx1 = 0, dy1 = 0;
    int dx2 = 0, dy2 = 0;
    switch (currentDirection) {
    case UP:    dy1 = offset1; dy2 = offset2; break;
    case DOWN:  dy1 = -offset1; dy2 = -offset2; break;
    case LEFT:  dx1 = offset1; dx2 = offset2; break;
    case RIGHT: dx1 = -offset1; dx2 = -offset2; break;
    }
    painter->save();
    painter->setOpacity(0.35);
    painter->drawPixmap(mainDrawPos.x() + dx1, mainDrawPos.y() + dy1, scaledSprite);
    painter->setOpacity(0.18);
    painter->drawPixmap(mainDrawPos.x() + dx2, mainDrawPos.y() + dy2, scaledSprite);
    painter->restore();
}

void PlayerTank::drawCooldownBar(QPainter* painter) const {
    float effective = reloadBoostTime > 0.0f ? 1.0f : shootCooldown;
    float denom = effective > 0.0f ? effective : 1.0f;
    float percent = lastShotTime / denom;
    if (percent < 0.0f) percent = 0.0f; 
    else if (percent > 1.0f) percent = 1.0f;
    QRectF barBg(position.x(), position.y() - 6.0, width, 4.0);
    painter->setBrush(QColor(60, 60, 60));
    painter->drawRect(barBg);
    QRectF barFg(position.x(), position.y() - 6.0, width * percent, 4.0);
    painter->setBrush(reloadBoostTime > 0.0f ? QColor(0, 200, 255) : QColor(80, 200, 120));
    painter->drawRect(barFg);
}

void PlayerTank::drawBoostBar(QPainter* painter, int& barY, float time, float duration, const QColor& color) const {
    if (time <= 0.0f || duration <= 0.0f) return;
    float progress = time / duration;
    if (progress < 0.0f) progress = 0.0f; else if (progress > 1.0f) progress = 1.0f;
    QRectF bg(position.x(), barY, width, 4.0);
    painter->setBrush(QColor(50, 50, 50));
    painter->drawRect(bg);
    QRectF fg(position.x(), barY, width * progress, 4.0);
    painter->setBrush(color);
    painter->drawRect(fg);
    barY -= 6;
}

void PlayerTank::resetControls() {
    isMoving = false;
    isShooting = false;
}

void PlayerTank::applySpeedBoost(float durationSeconds, float multiplier) {
    speedBoostTime = durationSeconds;
    speedBoostDuration = durationSeconds;
    speedMultiplier = multiplier > 0.0f ? multiplier : 1.0f;
}

void PlayerTank::applyReloadBoost(float durationSeconds) { reloadBoostTime = durationSeconds; reloadBoostDuration = durationSeconds; }

void PlayerTank::addShield() { if (shieldCharges < 1) shieldCharges = 1; }

bool PlayerTank::hasShield() const { return shieldCharges > 0; }

void PlayerTank::consumeShield() { if (shieldCharges > 0) shieldCharges -= 1; }

void PlayerTank::clearAllBuffs() {
    speedBoostTime = 0.0f;
    speedBoostDuration = 0.0f;
    speedMultiplier = 1.0f;
    reloadBoostTime = 0.0f;
    reloadBoostDuration = 0.0f;
    shieldCharges = 0;
    speed = baseSpeed;
}
