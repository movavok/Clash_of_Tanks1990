#ifndef TRAPABILITY_H
#define TRAPABILITY_H

#include "tank.h"

class TrapAbility : public Entity
{
public:
    TrapAbility(const QPointF&, Tank*);

    void update(float) override {};
    void render(QPainter*) override {};
    QRectF bounds() const override {};

private:
    Tank* owner;
    bool activated = false;
    float stunTime = 3.0f;
};

#endif // TRAPABILITY_H
