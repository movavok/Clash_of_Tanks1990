#include "powerup.h"

PowerUp::PowerUp(const QPointF& position, BoostType boostType)
    : Entity(position), type(boostType) {}

void PowerUp::update(float dt) {
    rotation += 90.0f * dt;
    if (rotation >= 360.0f) rotation -= 360.0f;
}

void PowerUp::render(QPainter* painter) {
	static QPixmap speedPx(":/powerups/speedPowerUp.png");
	static QPixmap reloadPx(":/powerups/reloadPowerUp.png");
	static QPixmap shieldPx(":/powerups/shieldPowerUp.png");
    static QPixmap trapPx(":/powerups/trapPowerUp.png");
	const QPixmap* sprite = nullptr;
	switch (type) {
    case BoostType::Speed:  sprite = &speedPx;  break;
    case BoostType::Reload: sprite = &reloadPx; break;
    case BoostType::Shield: sprite = &shieldPx; break;
    case BoostType::Trap: sprite = &trapPx; break;
	}
	if (sprite && !sprite->isNull()) {
		QPixmap scaled = sprite->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPointF center(position.x() + width / 2.0f, position.y() + height / 2.0f);
        painter->save();
        painter->translate(center);
        painter->rotate(rotation);
        painter->translate(-scaled.width() / 2.0f, -scaled.height() / 2.0f);
        painter->drawPixmap(0, 0, scaled);
        painter->restore();
    }
}

QRectF PowerUp::bounds() const { return QRectF(position.x(), position.y(), width, height); }

PowerUp::BoostType PowerUp::getType() const { return type; }
