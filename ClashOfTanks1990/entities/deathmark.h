#ifndef DEATHMARK_H
#define DEATHMARK_H

#include "entity.h"
#include <QPen>
#include <QColor>
#include <QPixmap>

class DeathMark : public Entity {
public:
    DeathMark(const QPointF&, float, float);
    void update(float) override;
    void render(QPainter*) override;
    QRectF bounds() const override;

private:
    float width;
    float height;
    float remaining;
    float duration;
};

#endif // DEATHMARK_H
