#include "tank.h"

Tank::Tank(const QPointF& pos, unsigned short wth, unsigned short hgt, float spd)
    : Entity(pos), width(wth), height(hgt), speed(spd), direction(UP) {}

void Tank::move(Direction dir, float dt) {
    float distance = speed * dt; //30 fps = speed * 0.033(30/1000 ms)

    setDirection(dir);
    switch(dir) {
        case UP: position.ry() -= distance; break;
        case DOWN: position.ry() += distance; break;
        case LEFT:  position.rx() -= distance; break;
        case RIGHT: position.rx() += distance; break;
    }
}

void Tank::setDirection(Direction dir) { direction = dir; }

QRectF Tank::bounds() const { return QRectF(position.x(), position.y(), width, height); }
