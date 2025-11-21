#ifndef TANK_H
#define TANK_H

#include "entity.h"

class Tank : public Entity
{
protected:
    unsigned short width, height;
    float speed;
    enum Direction { UP, DOWN, LEFT, RIGHT } direction;

public:
    Tank(const QPointF&, unsigned short, unsigned short, float);

    void move(Direction, float);
    virtual void shoot() = 0;
    void setDirection(Direction);
};

#endif // TANK_H
