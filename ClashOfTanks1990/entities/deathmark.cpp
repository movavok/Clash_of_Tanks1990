#include "deathmark.h"

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
    float progress = duration > 0.0f ? (remaining / duration) : 0.0f;
    if (progress < 0.0f) progress = 0.0f; else if (progress > 1.0f) progress = 1.0f;
    double alpha = progress; // 0..1 opacity

    static QPixmap markSprite(":/deathmark/deathMark.png");
    if (!markSprite.isNull()) {
        QPixmap scaled = markSprite.scaled(static_cast<int>(width), static_cast<int>(height), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const int drawX = static_cast<int>(position.x() + (width - scaled.width()) / 2.0f);
        const int drawY = static_cast<int>(position.y() + (height - scaled.height()) / 2.0f);
        painter->setOpacity(alpha);
        painter->drawPixmap(drawX, drawY, scaled);
    }

    painter->restore();
}

QRectF DeathMark::bounds() const { return QRectF(position.x(), position.y(), width, height); }
