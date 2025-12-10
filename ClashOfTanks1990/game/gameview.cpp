#include "gameview.h"

GameView::GameView(QWidget *parent)
    : QWidget{parent} {
    lastTime = QDateTime::currentMSecsSinceEpoch();
    timer.setInterval(16); // 1000/16 = ~60fps

    connect(&timer, &QTimer::timeout, this, &GameView::onTick);
    timer.start();

    setFocusPolicy(Qt::StrongFocus); // working keyboard
}

void GameView::onTick() {
    long long now = QDateTime::currentMSecsSinceEpoch();
    double dt = (now - lastTime) / 1000.0;

    game.update(dt);
    QRect bounds(0, 0, width(), height());
    //
    //should be wimdow collision
    //
    update();
}

void GameView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    game.render(&painter);
}

void GameView::keyPressEvent(QKeyEvent* event) { game.handleKeyPress(static_cast<Qt::Key>(event->key())); }

void GameView::keyReleaseEvent(QKeyEvent* event) { game.handleKeyRelease(static_cast<Qt::Key>(event->key())); }


