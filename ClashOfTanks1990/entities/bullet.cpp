#include "bullet.h"
#include "enemytank.h"

Bullet::Bullet(const QPointF& pos, Tank::Direction dir, float spd, Tank* own, BulletType type, float sizeCoef)
    : Entity(pos), direction(dir), speed(spd), owner(own)
{
    fromEnemy = (own && dynamic_cast<EnemyTank*>(own) != nullptr);
    switch (type) {
    case BulletType::Default: width = 8; height = 8; spritePath = ":/bullets/bullet.png"; break;
    case BulletType::Sniper: width = 10; height = 20; spritePath = ":/bullets/sniperBullet.png"; break;
    }
    width  = static_cast<unsigned short>(width * sizeCoef);
    height = static_cast<unsigned short>(height * sizeCoef);
}

void Bullet::update(float deltaTime) {
    float distance = speed * deltaTime;

    switch(direction) {
    case Tank::Direction::UP:    position.ry() -= distance; break;
    case Tank::Direction::DOWN:  position.ry() += distance; break;
    case Tank::Direction::LEFT:  position.rx() -= distance; break;
    case Tank::Direction::RIGHT: position.rx() += distance; break;
    }
}

void Bullet::render(QPainter* painter) {
    QPixmap sprite(spritePath);
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

unsigned short Bullet::getWidth() const { return width; }
unsigned short Bullet::getHeight() const { return height; }

bool Bullet::isFromEnemy() const { return fromEnemy; }

QRectF Bullet::bounds() const { return QRectF(position.x(), position.y(), width, height); }
