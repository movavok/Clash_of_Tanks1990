#include "entity.h"

Entity::Entity(const QPointF& pos) : position(pos) {}

void Entity::setPosition(const QPointF& pos) { position = pos; }

QPointF Entity::getPosition() const { return position; }

bool Entity::isAlive() const { return alive; }

void Entity::destroy() { alive = false; }
