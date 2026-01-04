#include "game.h"

Game::Game()
    : player(nullptr)
    , level(nullptr)
    , levelIndex(1)
    , advancing(false)
    , announcedNoEnemies(false)
{
    initLevel();
    detectMaxLevel();
    spawnPlayerAtTile(2, 2);
    spawnEnemiesDefault();
}

void Game::initLevel() {
    level = new Level(19, 19, 32);
    level->loadFromFile(":/levels/level1.txt");
    announcedNoEnemies = false;
}

void Game::detectMaxLevel() { maxLevel = QDir(":/levels").entryList(QStringList() << "*.txt", QDir::Files).size(); }
int Game::getMaxLevel() const { return maxLevel; }

void Game::newGame() { levelIndex = 1; restartLevel(); }

QPointF Game::tileCenter(int tileX, int tileY) const {
    if (!level) return QPointF(0, 0);
    const int tileSize = level->getTileSize();
    const int offset = (tileSize - 30) / 2;
    return QPointF(tileX * tileSize + offset, tileY * tileSize + offset);
}

void Game::spawnPlayerAtTile(int tileX, int tileY) {
    QPointF playerPos = tileCenter(tileX, tileY);
    player = new PlayerTank(playerPos, 30, 30, 100.0f);
    setMovementScheme(movementScheme, shootKey);
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

    bool eliteSpawned = false;

    for (const QPointF& position : enemySpawns) {
        bool spawnSniper = !eliteSpawned && QRandomGenerator::global()->bounded(100) < 45;
        bool spawnLaser = !eliteSpawned && QRandomGenerator::global()->bounded(100) < 70;
        EnemyTank* enemy = nullptr;

        if (spawnSniper) {
            enemy = new EnemySniper(position, player, &entities);
            eliteSpawned = true;
        } else if (spawnLaser) {
            enemy = new EnemyLaser(position, player, &entities);
            enemy->addShield();
            eliteSpawned = true;
        } else enemy = new EnemyTank(position, 30, 30, 100.0f, player, &entities);

        enemy->setTileSize(level->getTileSize());
        connect(enemy, &EnemyTank::bulletFired, this, &Game::addEntity);
        connect(enemy, &EnemyLaser::laserFired, this, &Game::onFireLaser);
        addEntity(enemy);
    }
}

bool Game::loadLevel(int index) {
    Audio::stopAll();

    Level* nextLevel = new Level(19, 19, 32);
    if (!nextLevel->loadFromFile(QString(":/levels/level%1.txt").arg(index))) {
        delete nextLevel;
        return false;
    }

    QList<Entity*> toRemove;
    for (Entity* &entity : entities)
        if (entity != player) toRemove.append(entity);

    for (Entity* entity : toRemove)
        removeEntity(entity);

    delete level;
    level = nextLevel;
    announcedNoEnemies = false;

    if (!player) spawnPlayerAtTile(2, 2);
    else {
        player->setPosition(tileCenter(2, 2));
        player->resetControls();
        player->clearAllBuffs();
    }

    spawnEnemiesDefault();
    emit levelChanged(levelIndex);
    return true;
}

void Game::advanceLevel()
{
    if (levelIndex >= maxLevel) return;
    ++levelIndex;
    if (!loadLevel(levelIndex)) --levelIndex;
}

void Game::restartLevel() { loadLevel(levelIndex); }

void Game::addEntity(Entity* entity) { entities.append(entity); }

void Game::removeEntity(Entity* entity) {
    int index = entities.indexOf(entity);
    if (index != -1) {
        entities.removeAt(index);
        if (Tank* removedTank = dynamic_cast<Tank*>(entity)) {
            for (Entity* &e : entities) {
                if (Bullet* bullet = dynamic_cast<Bullet*>(e))
                    if (bullet->getOwner() == removedTank) bullet->clearOwner();
            }
            QList<Entity*> lasersToRemove;
            for (Entity* &e : entities) {
                if (LaserRay* laser = dynamic_cast<LaserRay*>(e))
                    if (laser->getOwner() == removedTank) lasersToRemove.append(laser);
            }
            for (Entity* laserEntity : lasersToRemove) { removeEntity(laserEntity); Audio::stopSound("shootLaser"); }
        }

        if (QObject* obj = dynamic_cast<QObject*>(entity)) obj->disconnect();
        delete entity;
    }
}

void Game::update(float deltaTime, const QSize& windowSize) {
    if (paused) return;

    if (player && level)
        player->setHidden(level->intersectsAnyTiles(player->bounds(), { Level::TileType::Grass }));

    updateEntities(deltaTime, windowSize);
    powerUpSpawnTimer += deltaTime;
    int activePowerUps = 0;
    for (Entity* entity : entities) if (dynamic_cast<PowerUp*>(entity) && entity->isAlive()) ++activePowerUps;
    if (powerUpSpawnTimer >= powerUpSpawnInterval && activePowerUps < 2) {
        spawnPowerUpRandom();
        powerUpSpawnTimer = 0.0;
    }
    checkIfShotDown();

    for (Entity* &entity : entities) {
        PowerUp* boost = dynamic_cast<PowerUp*>(entity);
        if (!boost || !boost->isAlive()) continue;

        bool consumed = false;
        if (player && player->bounds().intersects(boost->bounds())) {
            applyPowerUp(boost);
            consumed = true;
        }
        if (!consumed) {
            for (Entity* &entity : entities) {
                if (EnemyTank* enemy = dynamic_cast<EnemyTank*>(entity)) {
                    if (enemy->isAlive() && enemy->bounds().intersects(boost->bounds())) {
                        switch (boost->getType()) {
                        case PowerUp::BoostType::Speed:  enemy->applySpeedBoost(8.0f, 1.5f); Audio::play("speedPowerUp"); break;
                        case PowerUp::BoostType::Reload: enemy->applyReloadBoost(8.0f); Audio::play("reloadPowerUp"); break;
                        case PowerUp::BoostType::Shield: enemy->addShield(); Audio::play("shieldPowerUp"); break;
                        }
                        boost->destroy();
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
    for (Entity* &entity : entities) {
        if (!entity->isAlive()) continue;

        QPointF oldPos = entity->getPosition();
        if (EnemyTank* enemy = dynamic_cast<EnemyTank*>(entity))
            enemy->setSeesPlayer(enemySeesPlayer(enemy));

        entity->update(deltaTime);
        if (LaserRay* laser = dynamic_cast<LaserRay*>(entity)) {
            if (level) {
                QRectF area = laser->bounds();
                level->destroyInRect(area);
                if (level->intersectsBulletSolid(area)) Audio::play("brickBreaking");
            }
        } else if (Bullet* bullet = dynamic_cast<Bullet*>(entity)) {
            if (bullet->isAlive() && checkWindowBounds(bullet, windowSize)) bullet->destroy();
            if (bullet->isAlive() && level->intersectsBulletSolid(bullet->bounds())) {
                level->destroyInRect(bullet->bounds());
                Audio::play("bulletToWall");
                bullet->destroy();
            }
            for (Entity* &other : entities) {
                if (!other->isAlive()) continue;
                if (DeathMark* corpse = dynamic_cast<DeathMark*>(other)) {
                    if (bullet->bounds().intersects(corpse->bounds())) { bullet->destroy(); break; }
                } else if (LaserRay* laser = dynamic_cast<LaserRay*>(other)) {
                    if (bullet->bounds().intersects(laser->bounds())) { bullet->applyLaserBoost(); break; }
                }
            }
        } else {
            bool collided = false;
            if (checkCollision(entity)) collided = true;
            if (!collided && level && level->intersectsTankSolid(entity->bounds())) collided = true;
            if (collided) entity->setPosition(oldPos);
        }
    }
}

void Game::onFireLaser(LaserRay* ray) {
    ray->setFullLength(calculateLaserLength(ray->getPosition(), ray->getDirection(), ray->getThickness()));
    addEntity(ray);
}

float Game::calculateLaserLength(const QPointF& origin, Tank::Direction dir, float thickness) const {
    if (!level) return 0.0f;

    const float step = 8.0f;
    float travelled = step * 0.5f;

    while (travelled <= 608.0f) {
        QRectF probeRect;
        switch (dir) {
        case Tank::Direction::RIGHT:
            probeRect = QRectF(origin.x() + travelled, origin.y(), step, thickness);
            break;
        case Tank::Direction::LEFT:
            probeRect = QRectF(origin.x() - travelled - step + thickness, origin.y(), step, thickness);
            break;
        case Tank::Direction::DOWN:
            probeRect = QRectF(origin.x(), origin.y() + travelled, thickness, step);
            break;
        case Tank::Direction::UP:
            probeRect = QRectF(origin.x(), origin.y() - travelled - step + thickness, thickness, step);
            break;
        }

        if (level->intersectsBulletSolid(probeRect)) { travelled += std::max(thickness, step); break; }

        bool hitTank = false;
        for (Entity* entity : entities) {
            if (!entity->isAlive()) continue;
            if (dynamic_cast<Bullet*>(entity) || dynamic_cast<PowerUp*>(entity) || dynamic_cast<LaserRay*>(entity))
                continue;
            if (probeRect.intersects(entity->bounds())) { hitTank = true; break; }
        }
        if (hitTank) { travelled += std::max(thickness, step); break; }

        travelled += step;
    }

    return travelled;
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
        Audio::play("lose");
        emit doPlayerDeathBox();
        return true;
    }
    return false;
}

void Game::handleLevelClear() {
    bool enemiesRemain = false;
    for (Entity* &enemy : entities)
        if (dynamic_cast<EnemyTank*>(enemy) && enemy->isAlive()) enemiesRemain = true;

    if (!enemiesRemain && !advancing) {
        advancing = true;
        paused = true;
        if (player) player->resetControls();

        QList<Entity*> bullets;
        for (Entity* &entity : entities) if (dynamic_cast<Bullet*>(entity)) bullets.append(entity);
        for (Entity* &bullet : bullets) removeEntity(bullet);

        emit doLevelChoiceBox(levelIndex);
    }
}

bool Game::enemySeesPlayer(const EnemyTank* enemy) const {
    return !level->checkBlockedLine(enemy->getPosition(), player->getPosition());
}

void Game::checkIfShotDown() {
    for (Entity* &entity : entities) {
        Bullet* bullet = dynamic_cast<Bullet*>(entity);
        if (!bullet || !bullet->isAlive()) continue;

        for (Entity* &target : entities) {
            if (bullet == target || !target->isAlive()) continue;

            if (Bullet* otherBullet = dynamic_cast<Bullet*>(target)) {
                if (otherBullet->isAlive() && bullet->bounds().intersects(otherBullet->bounds())) {
                    bullet->destroy();
                    otherBullet->destroy();
                    Audio::play("bulletToBullet");
                    break;
                }
            }
            else if (Tank* tank = dynamic_cast<Tank*>(target)) {
                if (tank == bullet->getOwner()) continue;

                bool ownerIsEnemy = bullet->isFromEnemy();
                bool targetIsEnemy = dynamic_cast<EnemyTank*>(tank);

                if (targetIsEnemy && ownerIsEnemy) {
                    if (bullet->bounds().intersects(tank->bounds())) bullet->destroy();
                    continue;
                }
                if (bullet->bounds().intersects(tank->bounds())) {
                    bullet->destroy();
                    if (PlayerTank* playerTankHit = dynamic_cast<PlayerTank*>(tank)) {
                        if (playerTankHit->hasShield()) { playerTankHit->consumeShield(); Audio::play("shieldDestroyed"); }
                        else tank->destroy();
                    } else if (EnemyTank* enemyTankHit = dynamic_cast<EnemyTank*>(tank)) {
                        if (enemyTankHit->hasShield()) { enemyTankHit->consumeShield(); Audio::play("shieldDestroyed"); }
                        else { tank->destroy(); Audio::play("tankDestroyed"); }
                    } else tank->destroy();
                    if (!tank->isAlive()) {
                        QRectF tankBounds = tank->bounds();
                        QPointF center = tankBounds.center();
                        float markSize = static_cast<float>(std::max(tankBounds.width(), tankBounds.height()));
                        addEntity(new DeathMark(center, markSize * 2, 0.125f, tank->getDirection()));
                    }
                    break;
                }
            }
        }
    }

    for (Entity* &entity : entities) {
        LaserRay* laser = dynamic_cast<LaserRay*>(entity);
        if (!laser || !laser->isAlive()) continue;

        if (player && player->isAlive() && laser->bounds().intersects(player->bounds())) {
            handleTankHit(player);
        }
    }
}

bool Game::checkCollision(Entity* entity) {
    if (dynamic_cast<Bullet*>(entity) || dynamic_cast<PowerUp*>(entity) || dynamic_cast<DeathMark*>(entity) || dynamic_cast<LaserRay*>(entity) || !entity->isAlive())
        return false;

    QRectF eBounds = entity->bounds();
    for (Entity* &other : entities) {
        if (other == entity || dynamic_cast<Bullet*>(other) || dynamic_cast<PowerUp*>(other) || dynamic_cast<LaserRay*>(other) || !other->isAlive())
            continue;
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
    for (Entity* &entity : entities) {
        if (!entity->isAlive()) continue;
        if (dynamic_cast<Bullet*>(entity)) continue;

        double opacity = originalOpacity;
        if (level->intersectsAnyTiles(entity->bounds(), { Level::TileType::Grass })) {
            if (entity == player) { opacity = 0.5; player->setHidden(true); }
            else if (dynamic_cast<EnemyTank*>(entity)) opacity = 0.0;
        }
        painter->setOpacity(opacity);
        entity->render(painter);
    }
    painter->setOpacity(originalOpacity);

    if (level) level->renderForeground(painter);

    for (Entity* &entity : entities) {
        if (!entity->isAlive()) continue;
        Bullet* bullet = dynamic_cast<Bullet*>(entity);
        DeathMark* mark = dynamic_cast<DeathMark*>(entity);
        if (!bullet && !mark) continue;
        painter->setOpacity(1.0);
        if (bullet) {
            painter->setOpacity(1.0);
            bullet->render(painter);
        }
        else if (mark) {
            painter->setOpacity(0.5);
            mark->render(painter);
        }
    }
    painter->setOpacity(originalOpacity);
}

void Game::handleKeyPress(Qt::Key key) { if (player) player->handleKeyPress(key); }

void Game::handleKeyRelease(Qt::Key key) { if (player) player->handleKeyRelease(key); }

void Game::setMovementScheme(int scheme, Qt::Key customShootKey) {
    movementScheme = scheme;
    shootKey = customShootKey;
    if (player) {
        if (scheme == 0) player->useWasdKeys();
        else if (scheme == 1) player->useArrowKeys();
        player->setKeyShoot(customShootKey);
    }
}

void Game::setPaused(bool p) { paused = p; resetPlayerControls(); }
bool Game::isPaused() const { return paused; }

void Game::advance() { advanceLevel(); }
void Game::finishBox() { paused = false; advancing = false; }

void Game::resetPlayerControls() { if (player) player->resetControls(); }

void Game::restart() { restartLevel(); }

void Game::spawnPowerUpRandom() {
    if (!level) return;

    const int tileSize = level->getTileSize();
    const int totalRows = level->getRows();
    const int totalCols = level->getCols();

    for (int attemptIndex = 0; attemptIndex < 24; ++attemptIndex) {
        const int tileX = QRandomGenerator::global()->bounded(totalCols);
        const int tileY = QRandomGenerator::global()->bounded(totalRows);

        const QPointF spawnPos(tileX * tileSize + (tileSize - 16) / 2.0,
                               tileY * tileSize + (tileSize - 16) / 2.0);
        const QRectF spawnRect(spawnPos.x(), spawnPos.y(), 16, 16);

        if (level->intersectsTankSolid(spawnRect)) continue;

        const PowerUp::BoostType chosenType = static_cast<PowerUp::BoostType>(QRandomGenerator::global()->bounded(3));
        PowerUp* boost = new PowerUp(spawnPos, chosenType);
        addEntity(boost);
        return;
    }
}

void Game::applyPowerUp(PowerUp* boost) {
    if (!player || !boost) return;
    switch (boost->getType()) {
    case PowerUp::BoostType::Speed: player->applySpeedBoost(8.0f, 1.5f); Audio::play("speedPowerUp"); break;
    case PowerUp::BoostType::Reload: player->applyReloadBoost(8.0f); Audio::play("reloadPowerUp"); break;
    case PowerUp::BoostType::Shield: player->addShield(); Audio::play("shieldPowerUp"); break;
    }
    boost->destroy();
}

void Game::handleTankHit(Tank* tank) {
    if (!tank || !tank->isAlive()) return;
    if (PlayerTank* playerTankHit = dynamic_cast<PlayerTank*>(tank)) {
        if (playerTankHit->hasShield()) { playerTankHit->consumeShield(); Audio::play("shieldDestroyed"); }
        else {
            tank->destroy(); Audio::play("tankDestroyed");
            QRectF tankBounds = tank->bounds();
            QPointF center = tankBounds.center();
            float markSize = static_cast<float>(std::max(tankBounds.width(), tankBounds.height()));
            addEntity(new DeathMark(center, markSize * 2, 0.125f, tank->getDirection()));
        }
    } else if (EnemyTank* enemyTankHit = dynamic_cast<EnemyTank*>(tank)) {
        if (enemyTankHit->hasShield()) { enemyTankHit->consumeShield(); Audio::play("shieldDestroyed"); }
        else { tank->destroy(); Audio::play("tankDestroyed");
            QRectF tankBounds = tank->bounds();
            QPointF center = tankBounds.center();
            float markSize = static_cast<float>(std::max(tankBounds.width(), tankBounds.height()));
            addEntity(new DeathMark(center, markSize * 2, 0.125f, tank->getDirection()));
        }
    } else {
        tank->destroy(); Audio::play("tankDestroyed");
        QRectF tankBounds = tank->bounds();
        QPointF center = tankBounds.center();
        float markSize = static_cast<float>(std::max(tankBounds.width(), tankBounds.height()));
        addEntity(new DeathMark(center, markSize * 2, 0.125f, tank->getDirection()));
    }
}
