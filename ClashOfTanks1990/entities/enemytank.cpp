#include "enemytank.h"

EnemyTank::EnemyTank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd)
    : Tank(pos, wth, hgt, spd)
{
    srand(static_cast<unsigned int>(time(nullptr))); //random generator
    baseSpeed = spd;
}

void EnemyTank::update(float deltaTime) {
    if (speedBoostTime > 0.0f) {
        speedBoostTime -= deltaTime;
        if (speedBoostTime <= 0.0f) { speedBoostTime = 0.0f; speedMultiplier = 1.0f; }
    }
    if (reloadBoostTime > 0.0f) {
        reloadBoostTime -= deltaTime;
        if (reloadBoostTime <= 0.0f) reloadBoostTime = 0.0f;
    }

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

void EnemyTank::render(QPainter* painter) {
    static QPixmap enemySprite("../../assets/tanks/enemyTank.png");
    drawShieldAura(painter);

    if (!enemySprite.isNull()) {
        QPixmap scaledSprite;
        QPoint drawPos = drawRotatedSprite(painter, enemySprite, scaledSprite);
        drawSpeedTrail(painter, drawPos, scaledSprite);
    } else {
        QColor tankColor = shieldCharges > 0 ? QColor(200, 60, 200) : QColor(220, 0, 0);
        painter->setBrush(tankColor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(bounds());
    }

    drawCooldownBar(painter);
}

void EnemyTank::drawShieldAura(QPainter* painter) const {
    if (!hasShield()) return;
    painter->save();
    const qreal shieldMargin = 6.0;
    QRectF shieldRect(position.x() - shieldMargin,
                      position.y() - shieldMargin,
                      width + shieldMargin * 2.0,
                      height + shieldMargin * 2.0);
    painter->setBrush(QColor(200, 60, 200, 110));
    painter->setPen(QPen(QColor(200, 60, 200, 160), 2.0));
    painter->drawEllipse(shieldRect);
    painter->restore();
}

QPoint EnemyTank::drawRotatedSprite(QPainter* painter, const QPixmap& sprite, QPixmap& outScaled) const {
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

void EnemyTank::drawSpeedTrail(QPainter* painter, const QPoint& mainDrawPos, const QPixmap& scaledSprite) const {
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

void EnemyTank::drawCooldownBar(QPainter* painter) const {
    float effective = reloadBoostTime > 0.0f ? 1.0f : shootCooldown;
    float denom = effective > 0.0f ? effective : 1.0f;
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
