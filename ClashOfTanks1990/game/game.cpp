#include "game.h"
#include <QRandomGenerator>
#include <algorithm>

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
    for (Entity* entity : entities) {
        if (entity != player) toRemove.append(entity);
    }
    for (Entity* entity : toRemove) removeEntity(entity);

    delete level;
    level = nextLevel;
    announcedNoEnemies = false;

    if (!player) spawnPlayerAtTile(2, 2);
    else {
        player->setPosition(tileCenter(2, 2));
        player->resetControls();
        // Clear buffs on level change
        player->clearAllBuffs();
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
    msg.setIcon(QMessageBox::NoIcon);
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
    updateEntities(deltaTime, windowSize);
    // Spawn power-ups periodically
    powerUpSpawnTimer += deltaTime;
    int activePowerUps = 0;
    for (Entity* e : entities) if (dynamic_cast<PowerUp*>(e) && e->isAlive()) ++activePowerUps;
    if (powerUpSpawnTimer >= powerUpSpawnInterval && activePowerUps < 2) {
        spawnPowerUpRandom();
        powerUpSpawnTimer = 0.0f;
    }
    checkIfShotDown();

    // Check pickups: player and enemies can take power-ups
    for (Entity* e : entities) {
        PowerUp* p = dynamic_cast<PowerUp*>(e);
        if (!p || !p->isAlive()) continue;

        bool consumed = false;
        if (player && player->bounds().intersects(p->bounds())) {
            applyPowerUp(p);
            consumed = true;
        }
        if (!consumed) {
            for (Entity* t : entities) {
                if (EnemyTank* et = dynamic_cast<EnemyTank*>(t)) {
                    if (et->isAlive() && et->bounds().intersects(p->bounds())) {
                        // Apply to enemy with same semantics, different visuals already handled in EnemyTank
                        switch (p->getType()) {
                        case PowerUp::Speed:  et->applySpeedBoost(8.0f, 1.5f); break;
                        case PowerUp::Reload: et->applyReloadBoost(8.0f); break;
                        case PowerUp::Shield: et->addShield(); break;
                        }
                        p->destroy();
                        consumed = true;
                        break;
                    }
                }
            }
        }
    }
    cleanupDeadEntities();
    if (handlePlayerDeath()) return;
    handleLevelClear();
}

void Game::updateEntities(float deltaTime, const QSize& windowSize) {
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
}

void Game::cleanupDeadEntities() {
    for (int index = entities.size() - 1; index >= 0; --index) {
        if (!entities[index]->isAlive()) {
            if (entities[index] == player) player = nullptr;
            removeEntity(entities[index]);
        }
    }
}

bool Game::handlePlayerDeath() {
    if (!player) {
        paused = true;
        // Ensure any lingering input is cleared
        if (player) player->resetControls();
        QMessageBox msg;
        msg.setIcon(QMessageBox::NoIcon);
        msg.setWindowTitle("Ви загинули");
        msg.setText("Перезапустити рівень чи вийти?");
        msg.setStandardButtons(QMessageBox::Retry | QMessageBox::Yes);
        if (QAbstractButton* retry = msg.button(QMessageBox::Retry)) retry->setText("Перезапустити рівень");
        if (QAbstractButton* yes = msg.button(QMessageBox::Yes)) yes->setText("Вийти");
        QMessageBox::StandardButton choice = static_cast<QMessageBox::StandardButton>(msg.exec());
        if (choice == QMessageBox::Retry) restartLevel();
        else QCoreApplication::quit();
        paused = false;
        return true;
    }
    return false;
}

void Game::handleLevelClear() {
    bool enemiesRemain = false;
    for (Entity* enemy : entities)
        if (dynamic_cast<EnemyTank*>(enemy) && enemy->isAlive()) { enemiesRemain = true; }

    if (!enemiesRemain && !advancing) {
        advancing = true;
        paused = true;
        if (player) player->resetControls();

        QList<Entity*> bullets;
        for (Entity* entity : entities) if (dynamic_cast<Bullet*>(entity)) bullets.append(entity);
        for (Entity* bullet : bullets) removeEntity(bullet);

        doMessage(levelIndex);

        paused = false;
        advancing = false;
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
                    if (bullet->bounds().intersects(tank->bounds())) bullet->destroy();
                    continue;
                }

                if (bullet->bounds().intersects(tank->bounds())) {
                    bullet->destroy();
                    if (PlayerTank* playerTankHit = dynamic_cast<PlayerTank*>(tank)) {
                        if (playerTankHit->hasShield()) {
                            playerTankHit->consumeShield();
                        } else {
                            tank->destroy();
                        }
                    } else if (EnemyTank* enemyTankHit = dynamic_cast<EnemyTank*>(tank)) {
                        if (enemyTankHit->hasShield()) {
                            enemyTankHit->consumeShield();
                        } else {
                            tank->destroy();
                        }
                    } else {
                        tank->destroy();
                    }
                    break;
                }
            }
        }
    }
}

bool Game::checkCollision(Entity* entity) {
    if (dynamic_cast<Bullet*>(entity) || dynamic_cast<PowerUp*>(entity) || !entity->isAlive()) return false;

    QRectF eBounds = entity->bounds();
    for (Entity* other : entities) {
        if (other == entity || dynamic_cast<Bullet*>(other) || dynamic_cast<PowerUp*>(other) || !other->isAlive()) continue;
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

    double originalOpacity = painter->opacity();
    for (Entity* entity : entities) {
        if (!entity->isAlive()) continue;

        double opacity = originalOpacity;
        if (level->intersectsGrass(entity->bounds())) {
            if (dynamic_cast<Bullet*>(entity)) opacity = 0.5;
            else if (entity == player) opacity = 0.5;
            else if (dynamic_cast<EnemyTank*>(entity)) opacity = 0.0;
        }
        painter->setOpacity(opacity);
        entity->render(painter);
    }
    painter->setOpacity(originalOpacity);

    if (level) level->renderForeground(painter);
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
    for (Entity* entity : entities)
        if (entity != player) toRemove.append(entity);

    for (Entity* entity : toRemove) removeEntity(entity);

    delete level;
    level = newLevel;
    announcedNoEnemies = false;

    if (!player) spawnPlayerAtTile(2, 2);
    else {
        player->setPosition(tileCenter(2, 2));
        player->resetControls();
        // Clear buffs on restart
        player->clearAllBuffs();
    }

    spawnEnemiesDefault();
}

void Game::spawnPowerUpRandom() {
    if (!level) return;

    const int ts = level->getTileSize();
    const int rows = level->getRows();
    const int cols = level->getCols();

    int centerX = cols / 2;
    int centerY = rows / 2;

    // Try a few times to find a non-solid tile near center
    for (int attempt = 0; attempt < 12; ++attempt) {
        int deltaTileX = QRandomGenerator::global()->bounded(-3, 4); // [-3,3]
        int deltaTileY = QRandomGenerator::global()->bounded(-3, 4);
        int targetTileX = std::max(0, std::min(cols - 1, centerX + deltaTileX));
        int targetTileY = std::max(0, std::min(rows - 1, centerY + deltaTileY));

        QPointF pos(targetTileX * ts + (ts - 16) / 2.0, targetTileY * ts + (ts - 16) / 2.0);
        QRectF rect(pos.x(), pos.y(), 16, 16);
        if (level->intersectsSolid(rect)) continue;

        PowerUp::Type type;
        int randomTypeIndex = QRandomGenerator::global()->bounded(3);
        if (randomTypeIndex == 0) type = PowerUp::Speed; else if (randomTypeIndex == 1) type = PowerUp::Reload; else type = PowerUp::Shield;

        PowerUp* p = new PowerUp(pos, type);
        addEntity(p);
        return;
    }
}

void Game::applyPowerUp(PowerUp* p) {
    if (!player || !p) return;
    switch (p->getType()) {
    case PowerUp::Speed:
        player->applySpeedBoost(8.0f, 1.5f);
        break;
    case PowerUp::Reload:
        player->applyReloadBoost(8.0f);
        break;
    case PowerUp::Shield:
        player->addShield();
        break;
    }
    p->destroy();
}
