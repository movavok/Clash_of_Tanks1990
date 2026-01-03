#include "laserray.h"
#include <QTransform>

LaserRay::LaserRay(const QPointF& startPos, Tank::Direction dir, float lifeTimeSeconds, float sizeCoef, Tank* ownerTank)
    : Entity(startPos)
    , direction(dir)
    , fullLength(0.0)
    , timeLeft(lifeTimeSeconds)
{
    thick = 16.0f * sizeCoef;
    segmentLen = thick;
    owner = ownerTank;

    static QPixmap spriteStart(":/bullets/laserRay/start.png");
    static QPixmap spriteMid(":/bullets/laserRay/mid.png");
    static QPixmap spriteEnd(":/bullets/laserRay/end.png");

    int angle = 0;
    switch (direction) {
    case Tank::Direction::RIGHT: angle = 0;   break;
    case Tank::Direction::DOWN:  angle = 90;  break;
    case Tank::Direction::LEFT:  angle = 180; break;
    case Tank::Direction::UP:    angle = 270; break;
    }
    QTransform rot; rot.rotate(angle);
    QPixmap rStart = spriteStart.transformed(rot, Qt::SmoothTransformation);
    QPixmap rMid   = spriteMid  .transformed(rot, Qt::SmoothTransformation);
    QPixmap rEnd   = spriteEnd  .transformed(rot, Qt::SmoothTransformation);
    sStart = rStart.scaled(static_cast<int>(segmentLen), static_cast<int>(thick), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    sMid   = rMid  .scaled(static_cast<int>(segmentLen), static_cast<int>(thick), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    sEnd   = rEnd  .scaled(static_cast<int>(segmentLen), static_cast<int>(thick), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void LaserRay::update(float dt) {
    if (currentLength < fullLength) {
        currentLength += growSpeed * dt;

        if (currentLength > fullLength)
            currentLength = fullLength;
    }
    timeLeft -= dt;
    pulseTime += dt;
    if (timeLeft <= 0.0f) destroy();
}

void LaserRay::render(QPainter* painter) {
    if (fullLength <= 0.0f) return;

    painter->save();

    QPointF dirVec;
    switch (direction) {
    case Tank::Direction::UP:    dirVec = QPointF(0, -1); break;
    case Tank::Direction::DOWN:  dirVec = QPointF(0,  1); break;
    case Tank::Direction::LEFT:  dirVec = QPointF(-1, 0); break;
    case Tank::Direction::RIGHT: dirVec = QPointF( 1, 0); break;
    }

    const int totalSegs = static_cast<int>(std::floor(currentLength / segmentLen));
    const int midSegs = std::max(totalSegs - 2, 0);

    painter->drawPixmap(static_cast<int>(position.x()), static_cast<int>(position.y()), sStart);

    QPointF cursor = position + dirVec * segmentLen;
    for (int index = 0; index < midSegs; ++index) {
        float phase = pulseTime * 10.0f - index * 0.4f;
        float alpha = 0.6f + 0.4f * std::sin(phase);

        painter->setOpacity(alpha);
        painter->drawPixmap(cursor, sMid);
        cursor += dirVec * segmentLen;
    }

    if (totalSegs >= 2)
        painter->drawPixmap(cursor, sEnd);

    painter->restore();
}

QRectF LaserRay::bounds() const {
    const float offset = thick * 1.0f / 3.0f;
    const float effectiveThick = std::max(thick - 2 * offset, 1.0f);

    switch (direction) {
    case Tank::Direction::RIGHT:
        return QRectF(position.x(), position.y() + offset, currentLength, effectiveThick);
    case Tank::Direction::LEFT:
        return QRectF(position.x() - currentLength + thick, position.y() + offset, currentLength, effectiveThick);
    case Tank::Direction::DOWN:
        return QRectF(position.x() + offset, position.y(), effectiveThick, currentLength);
    case Tank::Direction::UP:
        return QRectF(position.x() + offset, position.y() - currentLength + thick, effectiveThick, currentLength);
    }
    return QRectF(position.x(), position.y(), effectiveThick, effectiveThick);
}

void LaserRay::setFullLength(float length) { fullLength = length; }
float LaserRay::getThickness() const { return thick; }
float LaserRay::getSegmentLength() const { return segmentLen; }
Tank::Direction LaserRay::getDirection() const { return direction; }
Tank* LaserRay::getOwner() const { return owner; }
