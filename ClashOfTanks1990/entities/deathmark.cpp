#include "deathmark.h"

DeathMark::DeathMark(const QPointF& center, float size, float frameTimeSeconds, Tank::Direction direction)
    : Entity(QPointF(center.x() - size / 2.0f, center.y() - size / 2.0f))
    , width(size)
    , height(size)
    , frameTime(frameTimeSeconds)
    , lastDirection(direction)
{
    for (int i = 1; i <= 4; ++i) {
        QPixmap frame(QString(":/deathmark/burst/%1.png").arg(i));
        if (!frame.isNull()) frames.push_back(frame);
    }
}

void DeathMark::update(float deltaTime) {
    timer += deltaTime;

    if (timer >= frameTime) {
        timer -= frameTime;
        currentFrame++;
        if (currentFrame >= frames.size()) {
            frames.clear();
            state = RenderState::Corpse;
        }
    }
}

void DeathMark::render(QPainter* painter) {
    painter->save();

    if (state == RenderState::Burst) {
        QPixmap frame = frames[currentFrame].scaled(static_cast<int>(width), static_cast<int>(height), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int drawX = static_cast<int>(position.x() + (width - frame.width()) / 2.0f);
        int drawY = static_cast<int>(position.y() + (height - frame.height()) / 2.0f);
        painter->drawPixmap(drawX, drawY, frame);
    }
    else if (state == RenderState::Corpse) {
        static QPixmap corpse(QString(":/tanks/deadTank.png"));

        float rotation = 0.0f;
        switch (lastDirection) {
        case Tank::Direction::UP:    rotation = 0.0f;   break;
        case Tank::Direction::RIGHT: rotation = 90.0f;  break;
        case Tank::Direction::DOWN:  rotation = 180.0f; break;
        case Tank::Direction::LEFT:  rotation = 270.0f; break;
        }

        QPixmap scaled = corpse.scaled(static_cast<int>(width), static_cast<int>(height), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QTransform trans;
        trans.translate(position.x() + width / 2.0f, position.y() + height / 2.0f);
        trans.rotate(rotation);
        trans.translate(-scaled.width() / 2.0f, -scaled.height() / 2.0f);

        painter->setTransform(trans);
        painter->drawPixmap(0, 0, scaled);
    }
    painter->restore();
}

QRectF DeathMark::bounds() const { return QRectF(position.x(), position.y(), width, height); }
