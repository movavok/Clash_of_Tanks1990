#include "deathmark.h"
#include <QPen>
#include <QColor>
#include <QPixmap>
#include <QTransform>

DeathMark::DeathMark(const QPointF& center, float size, float lifetimeSeconds)
    : Entity(QPointF(center.x() - size / 2.0f, center.y() - size / 2.0f))
    , width(size)
    , height(size)
    , remaining(lifetimeSeconds)
    , duration(lifetimeSeconds)
{
}

void DeathMark::update(float deltaTime) {
    if (remaining > 0.0f) {
        remaining -= deltaTime;
        if (remaining <= 0.0f) { remaining = 0.0f; destroy(); }
    }
}

void DeathMark::render(QPainter* painter) {
    if (remaining <= 0.0f) return;
    painter->save();
    float pct = duration > 0.0f ? (remaining / duration) : 0.0f;
    if (pct < 0.0f) pct = 0.0f; else if (pct > 1.0f) pct = 1.0f;
    double alpha = pct; // 0..1 opacity

    static QPixmap markSprite(":/deathmark/deathMark.png");
    if (!markSprite.isNull()) {
        QPixmap scaled = markSprite.scaled(static_cast<int>(width), static_cast<int>(height), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // Center within our rect
        const int drawX = static_cast<int>(position.x() + (width - scaled.width()) / 2.0f);
        const int drawY = static_cast<int>(position.y() + (height - scaled.height()) / 2.0f);
        painter->setOpacity(alpha);
        painter->drawPixmap(drawX, drawY, scaled);
    } else {
        // Fallback to vector X
        int a = static_cast<int>(alpha * 200.0);
        QPen pen(QColor(255, 255, 255, a));
        pen.setWidth(3);
        painter->setPen(pen);
        const QPointF topLeft(position.x(), position.y());
        const QPointF topRight(position.x() + width, position.y());
        const QPointF bottomLeft(position.x(), position.y() + height);
        const QPointF bottomRight(position.x() + width, position.y() + height);
        painter->drawLine(topLeft, bottomRight);
        painter->drawLine(topRight, bottomLeft);
    }

    painter->restore();
}

QRectF DeathMark::bounds() const {
    return QRectF(position.x(), position.y(), width, height);
}
