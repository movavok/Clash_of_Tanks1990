#ifndef POWERUP_H
#define POWERUP_H

#include "entity.h"

class PowerUp : public Entity
{
public:
    enum class BoostType { Speed, Reload, Shield };

    PowerUp(const QPointF&, BoostType);

    void update(float) override {};
    void render(QPainter*) override;
    QRectF bounds() const override;

    BoostType getType() const;

private:
    unsigned short width = 16;
    unsigned short height = 16;
    BoostType type = BoostType::Speed;
};

#endif // POWERUP_H
