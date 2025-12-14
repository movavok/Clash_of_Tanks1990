#ifndef DEATHMARK_H
#define DEATHMARK_H

#include "entity.h"

class DeathMark : public Entity {
public:
    DeathMark(const QPointF& center, float size, float lifetimeSeconds);
    void update(float deltaTime) override;
    void render(QPainter* painter) override;
    QRectF bounds() const override;

private:
    float width;
    float height;
    float remaining;
    float duration;
};

#endif // DEATHMARK_H
