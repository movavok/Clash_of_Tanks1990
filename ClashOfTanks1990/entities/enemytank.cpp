#include "enemytank.h"

EnemyTank::EnemyTank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd, PlayerTank* player, const QList<Entity*>* list)
    : Tank(pos, wth, hgt, spd), player(player), entities(list), baseSpeed(spd) {
    lastSeenPos = pos;
}

void EnemyTank::setTileSize(int size) { tileSize = size; }
int EnemyTank::getTileSize() const { return tileSize; }

void EnemyTank::setSeesPlayer(bool flag) { seesPlayer = flag; }
void EnemyTank::setSeesBoost(bool flag) { seesBoost = flag; }
unsigned short EnemyTank::getViewRange() const { return viewRange; }

void EnemyTank::update(float deltaTime) {
    checkPrevPosition(deltaTime);
    updateBoosts(deltaTime);

    if (dodgeCooldown > 0.0f) dodgeCooldown -= deltaTime;
    decideBehavior(deltaTime);

    if (stuckTimer > 0.45f && currentIndicator() != IndicatorType::Aim && state != BehaviorState::Dodge) {
        currentDirection = unstuckDirection();
        stuckTimer = 0.0f;
        reactionTimer = 0.0f;
        isMoving = true;
    }
    if (stunTimer > 0.0f) {
        stunTimer -= deltaTime;
        isMoving = false;
        return;
    }

    if (isRecoiling) doRecoil(deltaTime);
    if (isMoving) move(currentDirection, deltaTime);

    if (!(bulletType == Bullet::BulletType::None)) tryShoot(deltaTime);
}

void EnemyTank::updateBoosts(float dt) {
    bool prevActive = reloadBoostWasActive;
    float prevReloadMul = reloadMultiplier;
    if (speedBoostTime > 0.0f) {
        speedBoostTime -= dt;
        if (speedBoostTime <= 0.0f) {
            speedBoostTime = 0.0f;
            speedMultiplier = 1.0f;
        }
    }

    speed = baseSpeed * speedMultiplier;

    if (reloadBoostTime > 0.0f) {
        reloadBoostTime -= dt;
        if (reloadBoostTime <= 0.0f) {
            reloadBoostTime = 0.0f;
            reloadMultiplier = 1.0f;
        }
    }

    bool nowActive = reloadBoostTime > 0.0f;
    if (!prevActive && nowActive && lastShotTime > 0.0f) {
        float prevMax = shootCooldown;
        float progress = lastShotTime / prevMax;
        progress = std::clamp(progress, 0.0f, 1.0f);

        float newMax = shootCooldown / reloadMultiplier;
        lastShotTime = newMax * progress;
    }
    if (prevActive && !nowActive && lastShotTime > 0.0f) {
        float prevMax = shootCooldown / prevReloadMul;
        float progress = lastShotTime / prevMax;
        progress = std::clamp(progress, 0.0f, 1.0f);

        float newMax = shootCooldown > 0.0f ? shootCooldown : 0.001f;
        lastShotTime = newMax * progress;
    }

    reloadBoostWasActive = nowActive;
}

void EnemyTank::decideBehavior(float dt) {
    reactionTimer += dt;

    if (dodgeCooldown <= 0.0f && bulletNearby()) {
        state = BehaviorState::Dodge;
        dodgeTimer = 0.25f;
        dodgeCooldown = 1.5f;
    }

    switch (state) {
    case BehaviorState::Patrol: patrolBehavior(dt); break;
    case BehaviorState::Chase: chaseBehavior(); break;
    case BehaviorState::Dodge: dodgeBehavior(dt); break;
    case BehaviorState::Collect: collectBehavior(); break;
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

    if (!canSeePlayer() && seesBoost) { state = BehaviorState::Collect; return; }

    if (behaviorTimer >= 1.0f && state == BehaviorState::Patrol) {
        currentDirection = static_cast<Direction>(rand() % 4);
        behaviorTimer = 0.0f;
    }

    isMoving = true;
}

void EnemyTank::chaseBehavior() {
    if (state == BehaviorState::Dodge) return;

    if (!canSeePlayer()) {
        if (reactionTimer > 0.5f) {
            state = seesBoost ? BehaviorState::Collect : BehaviorState::Patrol;
            reactionTimer = 0.0f;
        }
        return;
    }

    if (reactionTimer > 0.6f) {
        currentDirection = turnToPlayer();
        reactionTimer = 0.0f;
    }

}

void EnemyTank::dodgeBehavior(float dt) {
    dodgeTimer -= dt;
    if (dodgeTimer <= 0.0f) {
        speed = baseSpeed;
        state = BehaviorState::Chase;
        return;
    }

    speed = baseSpeed * dodgeSpeedCoef;
    QPointF perp1(-lastBulletDir.y(), lastBulletDir.x());
    QPointF perp2(lastBulletDir.y(), -lastBulletDir.x());

    QPointF pos1 = position + perp1;
    QPointF pos2 = position + perp2;

    float distance1 = (pos1.x() - lastBulletPos.x()) * (pos1.x() - lastBulletPos.x()) +
                      (pos1.y() - lastBulletPos.y()) * (pos1.y() - lastBulletPos.y());

    float distance2 = (pos2.x() - lastBulletPos.x()) * (pos2.x() - lastBulletPos.x()) +
                      (pos2.y() - lastBulletPos.y()) * (pos2.y() - lastBulletPos.y());

    QPointF evade = (distance1 > distance2) ? perp1 : perp2;
    if (std::abs(evade.x()) > std::abs(evade.y()))
        currentDirection = evade.x() > 0 ? Direction::RIGHT : Direction::LEFT;
    else
        currentDirection = evade.y() > 0 ? Direction::DOWN : Direction::UP;

    isMoving = true;
}

void EnemyTank::collectBehavior() {
    if (canSeePlayer()) { state = BehaviorState::Chase; return; }

    if (!seesBoost) { state = BehaviorState::Patrol; isMoving = true; return; }

    QPointF targetCenter;
    if (!nearestPowerUp(targetCenter)) { state = BehaviorState::Patrol; isMoving = true; return; }

    if (reactionTimer > 0.5f) {
        currentDirection = turnToPoint(targetCenter);
        reactionTimer = 0.0f;
    }
    isMoving = true;
}

PowerUp* EnemyTank::nearestPowerUp(QPointF& outCenter) const {
    PowerUp* nearest = nullptr;
    float bestDistance = 0.0f;
    for (Entity* entity : *entities) {
        if (PowerUp* boost = dynamic_cast<PowerUp*>(entity)) {
            if (!boost->isAlive()) continue;
            const QPointF center = boost->bounds().center();
            float deltaX = static_cast<float>(center.x() - position.x());
            float deltaY = static_cast<float>(center.y() - position.y());
            float maxViewRange = static_cast<float>(viewRange * tileSize);
            float distance = deltaX * deltaX + deltaY * deltaY;
            if (distance > pow(maxViewRange, 2)) continue;
            if (!nearest) { nearest = boost; outCenter = center; bestDistance = distance; }
            else if (distance < bestDistance) { bestDistance = distance; nearest = boost; outCenter = center; }
        }
    }
    return nearest;
}

Tank::Direction EnemyTank::turnToPoint(const QPointF& target) const {
    const float deltaX = static_cast<float>(target.x() - (position.x() + width  * 0.5f));
    const float deltaY = static_cast<float>(target.y() - (position.y() + height * 0.5f));
    if (std::abs(deltaX) > std::abs(deltaY))
        return deltaX > 0 ? Tank::Direction::RIGHT : Tank::Direction::LEFT;
    else
        return deltaY > 0 ? Tank::Direction::DOWN : Tank::Direction::UP;
}

void EnemyTank::checkPrevPosition(float deltaTime) {
    const float minMovementSquared = 0.25f;
    float deltaX = static_cast<float>(position.x() - lastSeenPos.x());
    float deltaY = static_cast<float>(position.y() - lastSeenPos.y());
    float movedSquared = deltaX * deltaX + deltaY * deltaY;
    if (movedSquared <= minMovementSquared) stuckTimer += deltaTime;
    else { stuckTimer = 0.0f; lastSeenPos = position; }
}

Tank::Direction EnemyTank::unstuckDirection() const {
    if (canSeePlayer() && player) {
        const float targetDeltaX = static_cast<float>(player->getPosition().x() - position.x());
        const float targetDeltaY = static_cast<float>(player->getPosition().y() - position.y());
        if (currentDirection == Tank::Direction::LEFT || currentDirection == Tank::Direction::RIGHT)
            return (targetDeltaY > 0) ? Tank::Direction::DOWN : Tank::Direction::UP;
        else
            return (targetDeltaX > 0) ? Tank::Direction::RIGHT : Tank::Direction::LEFT;
    }

    if (seesBoost) {
        QPointF boostCenter;
        if (nearestPowerUp(boostCenter)) {
            const float targetDeltaX = static_cast<float>(boostCenter.x() - position.x());
            const float targetDeltaY = static_cast<float>(boostCenter.y() - position.y());
            if (currentDirection == Tank::Direction::LEFT || currentDirection == Tank::Direction::RIGHT)
                return (targetDeltaY > 0) ? Tank::Direction::DOWN : Tank::Direction::UP;
            else
                return (targetDeltaX > 0) ? Tank::Direction::RIGHT : Tank::Direction::LEFT;
        }
    }

    if (currentDirection == Tank::Direction::LEFT || currentDirection == Tank::Direction::RIGHT)
        return (rand() % 2) ? Tank::Direction::UP : Tank::Direction::DOWN;
    else
        return (rand() % 2) ? Tank::Direction::LEFT : Tank::Direction::RIGHT;
}

bool EnemyTank::bulletNearby() {
    QRectF dangerZone = bounds().adjusted((-1) * reactionRange, (-1) * reactionRange, reactionRange, reactionRange);

    for (Entity* entity : *entities) {
        if (Bullet* bullet = dynamic_cast<Bullet*>(entity)) {
            if (!bullet->isAlive()) continue;
            if (bullet->isFromEnemy()) continue;
            if (dangerZone.intersects(bullet->bounds())) {
                lastBulletPos = bullet->bounds().center();
                lastBulletDir = bullet->getDirectionVector();
                return true;
            }
        }
    }
    return false;
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

    float cooldown = shootCooldown / reloadMultiplier;
    if (lastShotTime < cooldown) return;
    if (!canShoot()) return;

    Bullet* bullet = shoot();
    if (bullet) emit bulletFired(bullet);

    Audio::play(shootSoundID);
}

bool EnemyTank::canShoot() const { return true; }

bool EnemyTank::canSeePlayer() const {
    if (!player || player->ifHidden()) return false;
    if (!seesPlayer) return false;

    QPointF distance = player->getPosition() - position;
    const float maxRange = viewRange * tileSize;

    return (std::abs(distance.x()) < maxRange && std::abs(distance.y()) < maxRange);
}

Bullet* EnemyTank::shoot() {
    lastShotTime = 0.0f;
    if (chargeTimer > 0.0f) {
        lastCharge = chargeTimer;
        checkRecoilDirection();
        chargeTimer = 0.0f;
    }
    float sizeCoef = (reloadBoostTime > 0.0f) ? 1.6f : 1.0f;

    Bullet* bullet = new Bullet(QPointF(0, 0), currentDirection, bulletSpeed * bulletSpeedMult, this, bulletType, sizeCoef);

    float drawWth = (currentDirection == Direction::LEFT || currentDirection == Direction::RIGHT) ? bullet->getHeight() : bullet->getWidth();
    float drawHgt = (currentDirection == Direction::LEFT || currentDirection == Direction::RIGHT) ? bullet->getWidth() : bullet->getHeight();

    QPointF bulletPos;
    switch (currentDirection) {
    case Direction::UP:
        bulletPos = QPointF(position.x() + width/2 - drawWth/2, position.y() - drawHgt); break;
    case Direction::DOWN:
        bulletPos = QPointF(position.x() + width/2 - drawWth/2, position.y() + height); break;
    case Direction::LEFT:
        bulletPos = QPointF(position.x() - drawWth, position.y() + height/2 - drawHgt/2); break;
    case Direction::RIGHT:
        bulletPos = QPointF(position.x() + width, position.y() + height/2 - drawHgt/2); break;
    }
    bullet->setPosition(bulletPos);
    return bullet;
}

void EnemyTank::checkRecoilDirection() {
    float recoilDistance = 20.0f * chargeTimer;
    switch(currentDirection) {
    case Direction::UP: recoilY = recoilDistance; break;
    case Direction::DOWN: recoilY = -recoilDistance; break;
    case Direction::LEFT: recoilX = recoilDistance; break;
    case Direction::RIGHT: recoilX = -recoilDistance; break;
    }
    isRecoiling = true;
}

void EnemyTank::doRecoil(float dt) {
    float step = 200.0f * dt;

    if (recoilX != 0.0f) {
        float move = std::min(step, std::abs(recoilX));
        position.rx() += (recoilX > 0 ? move : -move);
        recoilX += (recoilX > 0 ? -move : move);
    }
    if (recoilY != 0.0f) {
        float move = std::min(step, std::abs(recoilY));
        position.ry() += (recoilY > 0 ? move : -move);
        recoilY += (recoilY > 0 ? -move : move);
    }

    if (recoilX == 0.0f && recoilY == 0.0f) isRecoiling = false;
}

void EnemyTank::render(QPainter* painter) {
    IndicatorType indicator = currentIndicator();

    QPixmap icon;
    switch (indicator) {
    case IndicatorType::Dodge: icon = QPixmap(":/indicators/dodgeBullets.png"); break;
    case IndicatorType::Chase: icon = QPixmap(":/indicators/canSeePlayer.png"); break;
    case IndicatorType::Aim: icon = QPixmap(":/indicators/aimAtPlayer.png"); break;
    case IndicatorType::Focus: icon = QPixmap(":/indicators/shootingLaser.png"); break;
    case IndicatorType::Gift: icon = QPixmap(":/indicators/collectingPowerUps.png"); break;
    default: break;
    }

    if (!icon.isNull()) {
        float iconSize = 16;
        QPixmap scaled = icon.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        if (indicator == IndicatorType::Aim) {
            QPointF center(position.x() + width / 2, position.y() + height + iconSize / 2 + 2);
            painter->save();
            painter->translate(center);
            float rotationAngle = chargeTimer * chargeTimer * 180.0f;
            painter->rotate(rotationAngle);
            painter->drawPixmap(-scaled.width() / 2, -scaled.height() / 2, scaled);
            painter->restore();
        } else painter->drawPixmap(position.x() + width / 2 - scaled.width() / 2, position.y() + height + 2, scaled);
    }

    QPixmap enemySprite(spritePath);
    drawShieldAura(painter);

    if (!enemySprite.isNull()) {
        QPixmap scaledSprite;
        QPoint drawPos = drawRotatedSprite(painter, enemySprite, scaledSprite);
        QPixmap drawSprite = (chargeTimer > 0.0f) ? redOverlay(scaledSprite)
                                                  : scaledSprite;

        painter->drawPixmap(drawPos, drawSprite);
        drawSpeedTrail(painter, drawPos, drawSprite);
    }

    drawCooldownBar(painter);

    int barY = static_cast<int>(position.y() - 12.0);
    drawBoostBar(painter, barY, speedBoostTime,  speedBoostDuration,  QColor(200, 90, 255));
    drawBoostBar(painter, barY, reloadBoostTime, reloadBoostDuration, QColor(247, 129, 32));
}

QPixmap EnemyTank::redOverlay(const QPixmap& source) {
    QColor redOverlay(255, 40, 40);
    redOverlay.setAlphaF(0.7f * (chargeTimer / 4.0f));

    QPixmap tinted(source.size());
    tinted.fill(Qt::transparent);

    QPainter overlayPainter(&tinted);
    overlayPainter.drawPixmap(0, 0, source);
    overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    overlayPainter.fillRect(tinted.rect(), redOverlay);
    overlayPainter.end();

    return tinted;
}

EnemyTank::IndicatorType EnemyTank::currentIndicator() const {
    if (state == BehaviorState::Dodge) return IndicatorType::Dodge;
    if (state == BehaviorState::Collect) return IndicatorType::Gift;
    if (canShowEye && canSeePlayer()) return IndicatorType::Chase;
    return IndicatorType::None;
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
    if (!isMoving && !isRecoiling) return;
    bool dodge = state == BehaviorState::Dodge;
    if (!(speedBoostTime > 0.0f || dodge || isRecoiling)) return;
    if (painter->opacity() <= 0.01) return;

    int recoilOffset = static_cast<int>(-18 * lastCharge / 4.0f);
    int offset1 = isRecoiling ? recoilOffset : dodge ? 6 : 10;
    int offset2 = isRecoiling ? 2 * recoilOffset : dodge ? 12 : 20;
    double opacity1 = dodge || isRecoiling ? 0.45 : 0.35;
    double opacity2 = dodge || isRecoiling ? 0.25 : 0.18;
    int dx1 = 0, dy1 = 0;
    int dx2 = 0, dy2 = 0;

    switch (currentDirection) {
    case Direction::UP:    dy1 = offset1; dy2 = offset2; break;
    case Direction::DOWN:  dy1 = -offset1; dy2 = -offset2; break;
    case Direction::LEFT:  dx1 = offset1; dx2 = offset2; break;
    case Direction::RIGHT: dx1 = -offset1; dx2 = -offset2; break;
    }
    qDebug() << "charge:" << lastCharge
             << "recoil:" << recoilOffset;
    painter->save();
    painter->setOpacity(opacity1);
    painter->drawPixmap(mainDrawPos.x() + dx1, mainDrawPos.y() + dy1, scaledSprite);
    painter->setOpacity(opacity2);
    painter->drawPixmap(mainDrawPos.x() + dx2, mainDrawPos.y() + dy2, scaledSprite);
    painter->restore();
}

void EnemyTank::drawCooldownBar(QPainter* painter) const {
    float effective = shootCooldown / reloadMultiplier;
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

void EnemyTank::applyReloadBoost(float durationSeconds, float multiplier) {
    reloadBoostTime = durationSeconds;
    reloadBoostDuration = durationSeconds;
    reloadMultiplier = multiplier;
}

void EnemyTank::addShield() { if (shieldCharges < 1) shieldCharges = 1; }

void EnemyTank::applyStun(float durationSeconds) { stunTimer = durationSeconds; }

bool EnemyTank::hasShield() const { return shieldCharges > 0; }

void EnemyTank::consumeShield() { if (shieldCharges > 0) shieldCharges -= 1; }

void EnemyTank::clearAllBuffs() {
    speedBoostTime = 0.0f;
    speedBoostDuration = 0.0f;
    speedMultiplier = 1.0f;
    reloadBoostTime = 0.0f;
    reloadBoostDuration = 0.0f;
    reloadMultiplier = 1.0f;
    shieldCharges = 0;
    speed = baseSpeed;
    stunTimer = 0.0f;
}
