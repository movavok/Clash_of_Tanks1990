#ifndef BULLET_H
#define BULLET_H

#include "tank.h"
#include "enemytank.h"

class Bullet : public Entity
{
public:
    Bullet(const QPointF&, Tank::Direction, float, Tank*, unsigned short);

    void update(float) override;
    void render(QPainter*) override;
    QRectF bounds() const override;

    Tank* getOwner() const;
    void clearOwner();
    bool isFromEnemy() const;
    static unsigned short getDefaultBulletSize() { return defaultSize; }

private:
    Tank* owner;
    Tank::Direction direction;
    float speed;
    unsigned short size;
    bool fromEnemy = false;
    static unsigned short defaultSize;
};

#endif // BULLET_H
