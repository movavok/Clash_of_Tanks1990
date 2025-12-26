#include "powerup.h"

PowerUp::PowerUp(const QPointF& position, BoostType boostType)
    : Entity(position), type(boostType) {}

void PowerUp::render(QPainter* painter) {
	static QPixmap speedPx(":/powerups/speedPowerUp.png");
	static QPixmap reloadPx(":/powerups/reloadPowerUp.png");
	static QPixmap shieldPx(":/powerups/shieldPowerUp.png");
	const QPixmap* sprite = nullptr;
	switch (type) {
    case BoostType::Speed:  sprite = &speedPx;  break;
    case BoostType::Reload: sprite = &reloadPx; break;
    case BoostType::Shield: sprite = &shieldPx; break;
	}
	if (sprite && !sprite->isNull()) {
		QPixmap scaled = sprite->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		const int drawX = static_cast<int>(position.x() + (width - scaled.width()) / 2.0f);
		const int drawY = static_cast<int>(position.y() + (height - scaled.height()) / 2.0f);
		painter->drawPixmap(drawX, drawY, scaled);
    }
}

QRectF PowerUp::bounds() const { return QRectF(position.x(), position.y(), width, height); }

PowerUp::BoostType PowerUp::getType() const { return type; }
