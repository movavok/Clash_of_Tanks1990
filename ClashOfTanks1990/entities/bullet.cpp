#include "bullet.h"
#include "enemytank.h"

Bullet::Bullet(const QPointF& pos, Tank::Direction dir, float spd, Tank* own, BulletType type, float sizeCoef)
    : Entity(pos), direction(dir), speed(spd), owner(own)
{
    fromEnemy = (own && dynamic_cast<EnemyTank*>(own) != nullptr);
    switch (type) {
    case BulletType::Default: width = 8; height = 8; spritePath = ":/bullets/bullet.png"; break;
    case BulletType::Sniper: width = 6; height = 18; spritePath = ":/bullets/sniperBullet.png"; break;
    case BulletType::None: break;
    }
    width  = static_cast<unsigned short>(width * sizeCoef);
    height = static_cast<unsigned short>(height * sizeCoef);
}

void Bullet::update(float deltaTime) {
    float distance = speed * speedMult * deltaTime;

    switch(direction) {
    case Tank::Direction::UP:    position.ry() -= distance; break;
    case Tank::Direction::DOWN:  position.ry() += distance; break;
    case Tank::Direction::LEFT:  position.rx() -= distance; break;
    case Tank::Direction::RIGHT: position.rx() += distance; break;
    }
}

void Bullet::render(QPainter* painter) {
    QPixmap sprite(laserBoosted ? ":/bullets/laserBullet.png" : spritePath);
    QPixmap scaled = sprite.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (!sprite.isNull()) {
        QTransform rot;
        switch (direction) {
        case Tank::Direction::UP:    rot.rotate(0);   break;
        case Tank::Direction::RIGHT: rot.rotate(90);  break;
        case Tank::Direction::DOWN:  rot.rotate(180); break;
        case Tank::Direction::LEFT:  rot.rotate(270); break;
        }
        QPixmap rotated = scaled.transformed(rot, Qt::SmoothTransformation);
        painter->drawPixmap(static_cast<int>(position.x()), static_cast<int>(position.y()), rotated);
    }
}

Tank* Bullet::getOwner() const { return owner; }
void Bullet::clearOwner() { owner = nullptr; }

Tank::Direction Bullet::getDirection() const { return direction; }

QPointF Bullet::getDirectionVector() const {
    switch (direction) {
    case Tank::Direction::UP:    return QPointF(0, -1);
    case Tank::Direction::DOWN:  return QPointF(0, 1);
    case Tank::Direction::LEFT:  return QPointF(-1, 0);
    case Tank::Direction::RIGHT: return QPointF(1, 0);
    }
    return QPointF(0,0);
}

unsigned short Bullet::getWidth() const { return width; }
unsigned short Bullet::getHeight() const { return height; }

bool Bullet::isFromEnemy() const { return fromEnemy; }

void Bullet::applyLaserBoost() {
    if (!laserBoosted) {
        laserBoosted = true;
        speedMult = 2.0f;
        width *= 1.6f;
        height *= 1.6f;
    }
}

bool Bullet::isLaserBoosted() const { return laserBoosted; }
QRectF Bullet::bounds() const { return QRectF(position.x(), position.y(), width, height); }
