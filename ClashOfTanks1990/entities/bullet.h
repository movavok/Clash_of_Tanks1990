#ifndef BULLET_H
#define BULLET_H

#include "tank.h"

class EnemyTank;

class Bullet : public Entity
{
public:
    enum class BulletType { Default, Sniper };

    Bullet(const QPointF&, Tank::Direction, float, Tank*, BulletType, float);

    void update(float) override;
    void render(QPainter*) override;
    QRectF bounds() const override;

    Tank* getOwner() const;
    void clearOwner();

    Tank::Direction getDirection() const;
    QPointF getDirectionVector() const;

    unsigned short getWidth() const;
    unsigned short getHeight() const;

    bool isFromEnemy() const;

private:
    Tank* owner;
    Tank::Direction direction;
    float speed;
    unsigned short width;
    unsigned short height;
    bool fromEnemy = false;
    QString spritePath;
};

#endif // BULLET_H
