#ifndef ENTITY_H
#define ENTITY_H

#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "../systems/audio.h"

class Entity
{
public:
    Entity(const QPointF&);
    virtual ~Entity() = default;

    virtual void update(float) = 0;
    virtual void render(QPainter*) = 0;
    virtual QRectF bounds() const = 0; //collision

    void setPosition(const QPointF&);
    QPointF getPosition() const;

    bool isAlive() const;
    void destroy();

protected:
    QPointF position;
    bool alive = true;
};

#endif // ENTITY_H
