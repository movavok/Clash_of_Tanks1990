#include "game.h"

Game::Game() {
    player = new PlayerTank(QPointF(100, 100), 30, 30, 100.0f);
    addEntity(player);

    for (int i = 0; i < 3; i++) {
        EnemyTank* enemy = new EnemyTank(QPointF(200 + i * 60, 200), 30, 30, 70.0f);
        addEntity(enemy);
    }
}

void Game::addEntity(Entity* e) { entities.append(e); }

void Game::removeEntity(Entity* e)
{
    entities.removeOne(e);
    delete e;
}

void Game::update(float deltaTime) {
    for (Entity* entity : entities) {
        QPointF oldPos = entity->getPosition();

        entity->update(deltaTime);

        if (checkCollision(entity)) entity->setPosition(oldPos); //otkat
    }

    for (int index = entities.size() - 1; index >= 0; --index)
        if (!entities[index]->isAlive()) removeEntity(entities[index]);
}

bool Game::checkCollision(Entity* entity) {
    QRectF eBounds = entity->bounds();
    for (Entity* other : entities) {
        if (other == entity) continue;
        if (eBounds.intersects(other->bounds())) return true;
    }
    return false;
}

void Game::render(QPainter* painter) { for (Entity* entity : entities) entity->render(painter); }

void Game::handleKeyPress(Qt::Key key) { if (player) player->handleKeyPress(key); }

void Game::handleKeyRelease(Qt::Key key) { if (player) player->handleKeyRelease(key); }
