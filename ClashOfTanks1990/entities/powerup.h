#ifndef POWERUP_H
#define POWERUP_H

#include "entity.h"

class PowerUp : public Entity
{
public:
    enum class BoostType { Speed, Reload, Shield, Trap };

    PowerUp(const QPointF&, BoostType);

    void update(float) override;
    void render(QPainter*) override;
    QRectF bounds() const override;

    BoostType getType() const;

private:
    unsigned short width = 26;
    unsigned short height = 26;
    BoostType type;
    float rotation = 0.0f;
};

#endif // POWERUP_H
