#include "game.h"

Game::Game()
    : player(nullptr)
    , level(nullptr)
    , levelIndex(1)
    , advancing(false)
    , announcedNoEnemies(false)
{
    initLevel();
    spawnPlayerAtTile(2, 2);
    spawnEnemiesDefault();
}

void Game::initLevel() {
    level = new Level(19, 19, 32);
    level->loadFromFile("../../assets/levels/level1.txt");
    announcedNoEnemies = false;
}

QPointF Game::tileCenter(int tx, int ty) const {
    if (!level) return QPointF(0, 0);
    const int ts = level->getTileSize();
    const int offset = (ts - 30) / 2;
    return QPointF(tx * ts + offset, ty * ts + offset);
}

void Game::spawnPlayerAtTile(int tx, int ty) {
    QPointF playerPos = tileCenter(tx, ty);
    player = new PlayerTank(playerPos, 30, 30, 100.0f);
    connect(player, &PlayerTank::bulletFired, this, &Game::addEntity);
    addEntity(player);
}

void Game::spawnEnemiesDefault() {
    if (!level) return;
    QList<QPointF> enemySpawns = {
        tileCenter(level->getCols() - 3, 2),
        tileCenter(2, level->getRows() - 3),
        tileCenter(level->getCols() - 3, level->getRows() - 3)
    };
    for (const QPointF& pos : enemySpawns) {
        EnemyTank* enemy = new EnemyTank(pos, 30, 30, 100.0f);
        connect(enemy, &EnemyTank::bulletFired, this, &Game::addEntity);
        addEntity(enemy);
    }

}

void Game::advanceLevel() {
    ++levelIndex;
    const QString nextPath = QString("../../assets/levels/level%1.txt").arg(levelIndex);

    Level* nextLevel = new Level(19, 19, 32);
    if (!nextLevel->loadFromFile(nextPath)) {
        --levelIndex;
        delete nextLevel;
        return;
    }

    QList<Entity*> toRemove;
    for (Entity* e : entities) {
        if (e != player) toRemove.append(e);
    }
    for (Entity* e : toRemove) removeEntity(e);

    delete level;
    level = nextLevel;
    announcedNoEnemies = false;

    if (!player) {
        spawnPlayerAtTile(2, 2);
    } else {
        player->setPosition(tileCenter(2, 2));
    }

    spawnEnemiesDefault();
}

void Game::addEntity(Entity* entity) { entities.append(entity); }

void Game::removeEntity(Entity* entity) {
    int idx = entities.indexOf(entity);
    if (idx != -1) {
        entities.removeAt(idx);
        if (Tank* removedTank = dynamic_cast<Tank*>(entity))
            for (Entity* entity : entities)
                if (Bullet* bullet = dynamic_cast<Bullet*>(entity))
                    if (bullet->getOwner() == removedTank) bullet->clearOwner();

        if (QObject* obj = dynamic_cast<QObject*>(entity)) obj->disconnect();
        delete entity;
    }
}

void Game::doMessage(int levelIndex) {
    QMessageBox msg;
    msg.setIcon(QMessageBox::Information);
    msg.setWindowTitle(levelIndex >= 3 ? "Вітаємо! Ви пройшли гру" : QString("Рівень %1 пройдено").arg(levelIndex));
    msg.setText(levelIndex >= 3 ? "Вийти або перезапустити рівень?" : "Перейти до наступного рівня?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Retry);
    if (QAbstractButton* yes = msg.button(QMessageBox::Yes)) yes->setText(levelIndex >= 3 ? "Вийти" : "Так");
    if (QAbstractButton* retry = msg.button(QMessageBox::Retry)) retry->setText("Перезапустити рівень");
    QMessageBox::StandardButton choice = static_cast<QMessageBox::StandardButton>(msg.exec());
    if (choice == QMessageBox::Yes) levelIndex >= 3 ? QCoreApplication::quit() : advanceLevel();
    else restartLevel();
}

void Game::update(float deltaTime, const QSize& windowSize) {
    if (paused) return;
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
        if (!entities[index]->isAlive()) {
            if (entities[index] == player) player = nullptr;
            removeEntity(entities[index]);
        }
    }

    bool enemiesRemain = false;
    for (Entity* enemy : entities) {
        if (dynamic_cast<EnemyTank*>(enemy) && enemy->isAlive()) { enemiesRemain = true; }
    }
    if (!enemiesRemain && !advancing) {
        advancing = true;
        paused = true;

        QList<Entity*> bullets;
        for (Entity* entity : entities) if (dynamic_cast<Bullet*>(entity)) bullets.append(entity);
        for (Entity* bullet : bullets) removeEntity(bullet);

        doMessage(levelIndex);

        paused = false;
        advancing = false;
        return;
    }
}

void Game::checkIfShotDown() {
    for (Entity* entity : entities) {
        Bullet* bullet = dynamic_cast<Bullet*>(entity);
        if (!bullet || !bullet->isAlive()) continue;

        for (Entity* target : entities) {
            if (bullet == target || !target->isAlive()) continue;

            if (Bullet* otherBullet = dynamic_cast<Bullet*>(target)) {
                if (otherBullet->isAlive() && bullet->bounds().intersects(otherBullet->bounds())) {
                    bullet->destroy();
                    otherBullet->destroy();
                    break;
                }
            }
            else if (Tank* tank = dynamic_cast<Tank*>(target)) {
                if (tank == bullet->getOwner()) continue;

                Tank* ownerTank = dynamic_cast<Tank*>(bullet->getOwner());
                bool ownerIsEnemy = ownerTank && dynamic_cast<EnemyTank*>(ownerTank);
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

void Game::restartLevel() {
    Level* newLevel = new Level(19, 19, 32);
    if (!newLevel->loadFromFile(QString("../../assets/levels/level%1.txt").arg(levelIndex))) {
        delete newLevel;
        return;
    }

    QList<Entity*> toRemove;
    for (Entity* entity : entities) {
        if (entity != player) toRemove.append(entity);
    }
    for (Entity* entity : toRemove) removeEntity(entity);

    delete level;
    level = newLevel;
    announcedNoEnemies = false;

    if (!player) spawnPlayerAtTile(2, 2);
    else player->setPosition(tileCenter(2, 2));

    spawnEnemiesDefault();
}
