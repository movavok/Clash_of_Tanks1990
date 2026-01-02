#ifndef TANK_H
#define TANK_H

#include <QObject>
#include "entity.h"

class Bullet;

class Tank : public QObject, public Entity
{
    Q_OBJECT
public:
    Tank(const QPointF&, unsigned short, unsigned short, float);

    enum class Direction { UP, DOWN, LEFT, RIGHT } direction;

    void move(Direction, float);
    virtual Bullet* shoot() = 0;
    QRectF bounds() const override;

    void setDirection(Direction);
    Direction getDirection() const;

protected:
    unsigned short width, height;
    float speed;
};

#endif // TANK_H
