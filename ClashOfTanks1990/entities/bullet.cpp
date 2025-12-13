#include "bullet.h"

unsigned short Bullet::size = 5;

Bullet::Bullet(const QPointF& pos, Tank::Direction dir, float spd, Tank* own)
    : Entity(pos), direction(dir), speed(spd), owner(own) {}

void Bullet::update(float deltaTime) {
    float distance = speed * deltaTime;

    switch(direction) {
    case Tank::UP:    position.ry() -= distance; break;
    case Tank::DOWN:  position.ry() += distance; break;
    case Tank::LEFT:  position.rx() -= distance; break;
    case Tank::RIGHT: position.rx() += distance; break;
    }
}

void Bullet::render(QPainter* painter) {
    painter->setBrush(Qt::yellow);
    painter->drawEllipse(bounds());
}

Tank* Bullet::getOwner() const { return owner; }

void Bullet::clearOwner() { owner = nullptr; }

QRectF Bullet::bounds() const { return QRectF(position.x(), position.y(), size, size); }
