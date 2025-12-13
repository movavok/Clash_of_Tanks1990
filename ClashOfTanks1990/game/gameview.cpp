#include "gameview.h"

GameView::GameView(QWidget *parent)
    : QWidget{parent} {
    lastTime = QDateTime::currentMSecsSinceEpoch();
    timer.setInterval(16); // 1000/16 = ~60fps

    connect(&timer, &QTimer::timeout, this, &GameView::onTick);
    timer.start();

    setFocusPolicy(Qt::StrongFocus); // working keyboard
    setFixedSize(19 * 32, 19 * 32);

    // Forward game level changes to whoever listens to GameView
    connect(&game, &Game::levelChanged, this, &GameView::levelChanged);
}

void GameView::onTick() {
    long long now = QDateTime::currentMSecsSinceEpoch();
    double dt = (now - lastTime) / 1000.0;

    lastTime = now;

    game.update(dt, size());
    QRect bounds(0, 0, width(), height());
    update();
}

void GameView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    game.render(&painter);
}

void GameView::keyPressEvent(QKeyEvent* event) { game.handleKeyPress(static_cast<Qt::Key>(event->key())); }

void GameView::keyReleaseEvent(QKeyEvent* event) { game.handleKeyRelease(static_cast<Qt::Key>(event->key())); }

