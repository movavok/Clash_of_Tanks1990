#include "enemytank.h"

EnemyTank::EnemyTank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd, PlayerTank* player)
    : Tank(pos, wth, hgt, spd), player(player), baseSpeed(spd) {}

void EnemyTank::setTileSize(int size) { tileSize = size; }

void EnemyTank::setSeesPlayer(bool flag) { seesPlayer = flag; }

void EnemyTank::update(float deltaTime) {
    updateBoosts(deltaTime);

    decideBehavior(deltaTime);

    if (isMoving) move(currentDirection, deltaTime);

    tryShoot(deltaTime);
}

void EnemyTank::updateBoosts(float dt) {
    if (speedBoostTime > 0.0f) {
        speedBoostTime -= dt;
        if (speedBoostTime <= 0.0f) {
            speedBoostTime = 0.0f;
            speedMultiplier = 1.0f;
        }
    }
    if (reloadBoostTime > 0.0f) {
        reloadBoostTime -= dt;
        if (reloadBoostTime <= 0.0f) reloadBoostTime = 0.0f;
    }

    speed = baseSpeed * speedMultiplier;
}

void EnemyTank::decideBehavior(float dt) {
    reactionTimer += dt;

    switch (state) {
    case BehaviorState::Patrol: patrolBehavior(dt); break;
    case BehaviorState::Chase: chaseBehavior(dt); break;
    }
}

void EnemyTank::patrolBehavior(float dt) {
    behaviorTimer += dt;

    if (canSeePlayer())
        if (reactionTimer > 0.6) {
            currentDirection = turnToPlayer();
            reactionTimer = 0.0f;
            state = BehaviorState::Chase;
            return;
        }

    if (behaviorTimer >= 1.0f && state == BehaviorState::Patrol) {
        currentDirection = static_cast<Direction>(rand() % 4);
        behaviorTimer = 0.0f;
    }

    isMoving = true;
}

void EnemyTank::chaseBehavior(float dt) {
    if (!canSeePlayer()) {
        if (reactionTimer > 0.5f) {
            state = BehaviorState::Patrol;
            reactionTimer = 0.0f;
        }
        return;
    }

    if (reactionTimer > 0.6f) {
        currentDirection = turnToPlayer();
        reactionTimer = 0.0f;
    }

}

Tank::Direction EnemyTank::turnToPlayer() const {
    if (!player) return currentDirection;
    float dx = player->getPosition().x() - position.x();
    float dy = player->getPosition().y() - position.y();

    if (std::abs(dx) > std::abs(dy)) 
        return dx > 0 ? Tank::Direction::RIGHT : Tank::Direction::LEFT;
    else 
        return dy > 0 ? Tank::Direction::DOWN : Tank::Direction::UP;
}

void EnemyTank::tryShoot(float dt) {
    lastShotTime += dt;

    float cooldown = reloadBoostTime > 0.0f ? 1.0f : shootCooldown;
    if (lastShotTime < cooldown) return;

    Bullet* bullet = shoot();
    if (bullet) emit bulletFired(bullet);

    Audio::play("shoot");
}

bool EnemyTank::canSeePlayer() const {
    if (!player || player->ifHidden()) return false;
    if (!seesPlayer) return false;

    QPointF distance = player->getPosition() - position;
    const float maxRange = 6 * tileSize;

    return (std::abs(distance.x()) < maxRange && std::abs(distance.y()) < maxRange);
}

Bullet* EnemyTank::shoot() {
    lastShotTime = 0.0f;
    QPointF bulletPos;
    unsigned short currentBulletSize = reloadBoostTime > 0.0f ? 12 : Bullet::getDefaultBulletSize();

    switch (currentDirection) {
    case Direction::UP:
        bulletPos = QPointF(position.x() + width/2 - currentBulletSize/2, position.y() - currentBulletSize); break;
    case Direction::DOWN:
        bulletPos = QPointF(position.x() + width/2 - currentBulletSize/2, position.y() + height); break;
    case Direction::LEFT:
        bulletPos = QPointF(position.x() - currentBulletSize, position.y() + height/2 - currentBulletSize/2); break;
    case Direction::RIGHT:
        bulletPos = QPointF(position.x() + width, position.y() + height/2 - currentBulletSize/2); break;
    }
    return new Bullet(bulletPos, currentDirection, 150.0f, this, currentBulletSize);
}

void EnemyTank::render(QPainter* painter) {
    static QPixmap enemySprite(":/tanks/enemyTank.png");
    drawShieldAura(painter);

    if (!enemySprite.isNull()) {
        QPixmap scaledSprite;
        QPoint drawPos = drawRotatedSprite(painter, enemySprite, scaledSprite);
        drawSpeedTrail(painter, drawPos, scaledSprite);
    }

    drawCooldownBar(painter);

    // Boost duration bars
    int barY = static_cast<int>(position.y() - 12.0);
    drawBoostBar(painter, barY, speedBoostTime,  speedBoostDuration,  QColor(200, 90, 255));
    drawBoostBar(painter, barY, reloadBoostTime, reloadBoostDuration, QColor(247, 129, 32));
}

void EnemyTank::drawShieldAura(QPainter* painter) const {
    if (!hasShield()) return;
    painter->save();
    const double shieldMargin = 6.0;
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
    case Direction::UP:    angleDeg = 0;   break;
    case Direction::RIGHT: angleDeg = 90;  break;
    case Direction::DOWN:  angleDeg = 180; break;
    case Direction::LEFT:  angleDeg = 270; break;
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
    if (painter->opacity() <= 0.01) return;
    const int offset1 = 10;
    const int offset2 = 20;
    int dx1 = 0, dy1 = 0;
    int dx2 = 0, dy2 = 0;
    switch (currentDirection) {
    case Direction::UP:    dy1 = offset1; dy2 = offset2; break;
    case Direction::DOWN:  dy1 = -offset1; dy2 = -offset2; break;
    case Direction::LEFT:  dx1 = offset1; dx2 = offset2; break;
    case Direction::RIGHT: dx1 = -offset1; dx2 = -offset2; break;
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
    if (percent < 0.0f) percent = 0.0f;
    else if (percent > 1.0f) percent = 1.0f;
    QRectF barBg(position.x(), position.y() - 6.0, width, 4.0);
    painter->setBrush(QColor(60, 60, 60));
    painter->drawRect(barBg);
    QRectF barFg(position.x(), position.y() - 6.0, width * percent, 4.0);
    painter->setBrush(reloadBoostTime > 0.0f ? QColor(235, 61, 139) : QColor(240, 180, 60));
    painter->drawRect(barFg);
}

void EnemyTank::drawBoostBar(QPainter* painter, int& barY, float time, float duration, const QColor& color) const {
    if (time <= 0.0f || duration <= 0.0f) return;
    float progress = time / duration;
    if (progress < 0.0f) progress = 0.0f;
    else if (progress > 1.0f) progress = 1.0f;
    QRectF bg(position.x(), barY, width, 4.0);
    painter->setBrush(QColor(50, 50, 50));
    painter->drawRect(bg);
    QRectF fg(position.x(), barY, width * progress, 4.0);
    painter->setBrush(color);
    painter->drawRect(fg);
    barY -= 6;
}

void EnemyTank::applySpeedBoost(float durationSeconds, float multiplier) {
    speedBoostTime = durationSeconds;
    speedBoostDuration = durationSeconds;
    speedMultiplier = multiplier > 0.0f ? multiplier : 1.0f;
}

void EnemyTank::applyReloadBoost(float durationSeconds) {
    reloadBoostTime = durationSeconds;
    reloadBoostDuration = durationSeconds;
}

void EnemyTank::addShield() { if (shieldCharges < 1) shieldCharges = 1; }

bool EnemyTank::hasShield() const { return shieldCharges > 0; }

void EnemyTank::consumeShield() { if (shieldCharges > 0) shieldCharges -= 1; }

void EnemyTank::clearAllBuffs() {
    speedBoostTime = 0.0f;
    speedBoostDuration = 0.0f;
    speedMultiplier = 1.0f;
    reloadBoostTime = 0.0f;
    reloadBoostDuration = 0.0f;
    shieldCharges = 0;
    speed = baseSpeed;
}
