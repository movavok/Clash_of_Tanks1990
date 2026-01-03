#ifndef LASERRAY_H
#define LASERRAY_H

#include "tank.h"
#include <QPixmap>

class LaserRay : public Entity {
public:
    LaserRay(const QPointF&, Tank::Direction, float, float, Tank*);

    void update(float) override;
    void render(QPainter*) override;
    QRectF bounds() const override;

    void setFullLength(float);
    float getThickness() const;
    float getSegmentLength() const;
    Tank::Direction getDirection() const;
    Tank* getOwner() const;

private:
    Tank::Direction direction;
    float fullLength;
    float thick;
    float segmentLen;
    float timeLeft;
    Tank* owner = nullptr;

    float growSpeed = 1200.0f; //px/sec
    float currentLength = 0.0f;
    float pulseTime = 0.0f;

    QPixmap sStart;
    QPixmap sMid;
    QPixmap sEnd;
};

#endif // LASERRAY_H
