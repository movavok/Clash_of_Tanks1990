#include "tank.h"

Tank::Tank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd)
    : Entity(pos), direction(Direction::UP), width(wth), height(hgt), speed(spd) {}

void Tank::move(Direction dir, float dt) {
    float distance = speed * dt; //30 fps = speed * 0.033(30/1000 ms)

    setDirection(dir);
    switch(dir) {
        case Direction::UP: position.ry() -= distance; break;
        case Direction::DOWN: position.ry() += distance; break;
        case Direction::LEFT:  position.rx() -= distance; break;
        case Direction::RIGHT: position.rx() += distance; break;
    }
}

void Tank::setDirection(Direction dir) { direction = dir; }
Tank::Direction Tank::getDirection() const { return direction; }

QRectF Tank::bounds() const { return QRectF(position.x(), position.y(), width, height); }
