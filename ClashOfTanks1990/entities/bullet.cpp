#include "bullet.h"

unsigned short Bullet::defaultSize = 5;

Bullet::Bullet(const QPointF& pos, Tank::Direction dir, float spd, Tank* own, unsigned short sz)
    : Entity(pos), direction(dir), speed(spd), owner(own), size(sz) {}

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
    static QPixmap sprite(":/bullets/bullet.png");
    if (!sprite.isNull()) {
        QTransform rot;
        switch (direction) {
        case Tank::UP:    rot.rotate(0);   break;
        case Tank::RIGHT: rot.rotate(90);  break;
        case Tank::DOWN:  rot.rotate(180); break;
        case Tank::LEFT:  rot.rotate(270); break;
        }
        QPixmap rotated = sprite.transformed(rot, Qt::SmoothTransformation);
        QPixmap scaled = rotated.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->drawPixmap(static_cast<int>(position.x()), static_cast<int>(position.y()), scaled);
    } else { painter->setBrush(Qt::yellow); painter->drawEllipse(bounds()); }
}

Tank* Bullet::getOwner() const { return owner; }

void Bullet::clearOwner() { owner = nullptr; }

QRectF Bullet::bounds() const { return QRectF(position.x(), position.y(), size, size); }
