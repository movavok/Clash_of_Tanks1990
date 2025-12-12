#include "game.h"

Game::Game() {
    player = new PlayerTank(QPointF(100, 100), 30, 30, 100.0f);
    connect(player, &PlayerTank::bulletFired, this, &Game::addEntity);
    addEntity(player);

    for (int index = 0; index < 3; index++) {
        EnemyTank* enemy = new EnemyTank(QPointF(200 + index * 60, 200), 30, 30, 100.0f);
        connect(enemy, &EnemyTank::bulletFired, this, &Game::addEntity);
        addEntity(enemy);
    }

    level = new Level(25, 19, 32);
}

void Game::addEntity(Entity* entity) { entities.append(entity); }

void Game::removeEntity(Entity* entity) {
    entities.removeOne(entity);
    delete entity;
}

void Game::update(float deltaTime, const QSize& windowSize) {
    for (Entity* entity : entities) {
        if (!entity->isAlive()) continue;

        QPointF oldPos = entity->getPosition();
        entity->update(deltaTime);

        if (Bullet* bullet = dynamic_cast<Bullet*>(entity)) {
            if (bullet->isAlive() && checkWindowBounds(bullet, windowSize)) bullet->destroy();
            if (bullet->isAlive() && level && level->intersectsSolid(bullet->bounds())) {
                level->destroyInRect(bullet->bounds());
                bullet->destroy();
            }
        } else {
            bool collided = false;
            if (checkCollision(entity)) collided = true;
            if (!collided && level && level->intersectsSolid(entity->bounds())) collided = true;
            if (collided) entity->setPosition(oldPos);
        }
    }

    checkIfShotDown();

    for (int index = entities.size() - 1; index >= 0; --index) {
        if (!entities[index]->isAlive())
            removeEntity(entities[index]);
    }
}

void Game::checkIfShotDown() {
    for (Entity* entity : entities) {
        Bullet* bullet = dynamic_cast<Bullet*>(entity);
        if (!bullet || !bullet->isAlive()) continue;

        for (Entity* target : entities) {
            if (bullet == target || !target->isAlive()) continue;

            // bullet vs bullet
            if (Bullet* otherBullet = dynamic_cast<Bullet*>(target)) {
                if (otherBullet->isAlive() && bullet->bounds().intersects(otherBullet->bounds())) {
                    bullet->destroy();
                    otherBullet->destroy();
                    break;
                }
            }
            // bullet vs tank
            else if (Tank* tank = dynamic_cast<Tank*>(target)) {
                if (tank == bullet->getOwner()) continue;

                bool ownerIsEnemy = dynamic_cast<EnemyTank*>(bullet->getOwner());
                bool targetIsEnemy = dynamic_cast<EnemyTank*>(tank);

                if (targetIsEnemy && ownerIsEnemy) {
                    if (bullet->bounds().intersects(tank->bounds()))
                        bullet->destroy();
                    continue;
                }

                if (bullet->bounds().intersects(tank->bounds())) {
                    bullet->destroy();
                    tank->destroy();
                    break;
                }
            }
        }
    }
}

bool Game::checkCollision(Entity* entity) {
    if (dynamic_cast<Bullet*>(entity) || !entity->isAlive()) return false;

    QRectF eBounds = entity->bounds();
    for (Entity* other : entities) {
        if (other == entity || dynamic_cast<Bullet*>(other) || !other->isAlive()) continue;
        if (eBounds.intersects(other->bounds())) return true;
    }
    return false;
}

bool Game::checkWindowBounds(Entity* entity, const QSize& windowSize) {
    QRectF eBounds = entity->bounds();
    return eBounds.left() < 0 || eBounds.top() < 0 || eBounds.right() > windowSize.width() || eBounds.bottom() > windowSize.height();
}

void Game::render(QPainter* painter) {
    if (level) level->render(painter);
    for (Entity* entity : entities)
        if (entity->isAlive()) entity->render(painter);
}

void Game::handleKeyPress(Qt::Key key) { if (player) player->handleKeyPress(key); }

void Game::handleKeyRelease(Qt::Key key) { if (player) player->handleKeyRelease(key); }
