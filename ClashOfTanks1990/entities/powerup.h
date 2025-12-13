#ifndef POWERUP_H
#define POWERUP_H

#include "entity.h"

class PowerUp : public Entity
{
public:
    enum Type { Speed, Reload, Shield };

    PowerUp(const QPointF& pos, Type t);

    void update(float) override;
    void render(QPainter*) override;

    QRectF bounds() const override;

    Type getType() const { return type; }

private:
    unsigned short width = 16;
    unsigned short height = 16;
    Type type = Speed;
};

#endif // POWERUP_H
