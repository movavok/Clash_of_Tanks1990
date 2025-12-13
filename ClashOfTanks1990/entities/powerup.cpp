#include "powerup.h"

PowerUp::PowerUp(const QPointF& position, Type boostType)
    : Entity(position), type(boostType) {}

void PowerUp::render(QPainter* painter) {
	QColor color;
	switch (type) {
	case Speed:  color = QColor(80, 160, 255); break;
	case Reload: color = QColor(255, 200, 60); break;
	case Shield: color = QColor(120, 220, 120); break;
	}
	painter->setBrush(color);
	painter->setPen(Qt::NoPen);
	painter->drawRect(bounds());
}

QRectF PowerUp::bounds() const { return QRectF(position.x(), position.y(), width, height); }

PowerUp::Type PowerUp::getType() const { return type; }
