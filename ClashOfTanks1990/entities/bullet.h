#ifndef BULLET_H
#define BULLET_H

#include "tank.h"

class Bullet : public Entity
{
public:
    Bullet(const QPointF&, Tank::Direction, float, Tank*);

    void update(float) override;
    void render(QPainter*) override;
    QRectF bounds() const override;

    Tank* getOwner() const;
    void clearOwner();
    static unsigned short getBulletSize() { return size; }

private:
    Tank* owner;
    Tank::Direction direction;
    float speed;
    static unsigned short size;
};

#endif // BULLET_H
