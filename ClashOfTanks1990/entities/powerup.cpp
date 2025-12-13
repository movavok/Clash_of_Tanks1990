#include "powerup.h"
#include <QPainter>

PowerUp::PowerUp(const QPointF& pos, Type t)
	: Entity(pos), type(t) {}

void PowerUp::update(float) {
	// Static collectible; no behavior required.
}

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

QRectF PowerUp::bounds() const {
	return QRectF(position.x(), position.y(), width, height);
}

