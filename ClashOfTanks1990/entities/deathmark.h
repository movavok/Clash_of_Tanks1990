#ifndef DEATHMARK_H
#define DEATHMARK_H

#include "tank.h"
#include <QPen>
#include <QColor>
#include <QPixmap>

class DeathMark : public Entity {
public:
    enum class RenderState { Burst, Corpse };

    DeathMark(const QPointF&, float, float, Tank::Direction);
    void update(float) override;
    void render(QPainter*) override;
    QRectF bounds() const override;

private:
    float width;
    float height;

    float collisionScale = 0.5f;

    QVector<QPixmap> frames;
    int currentFrame = 0;
    float frameTime;
    float timer = 0.0f;

    RenderState state = RenderState::Burst;
    Tank::Direction lastDirection;
};

#endif // DEATHMARK_H
