#include "gameview.h"

GameView::GameView(QWidget *parent)
    : QWidget{parent} {
    lastTime = QDateTime::currentMSecsSinceEpoch();
    timer.setInterval(16); // 1000/16 = ~60fps

    connect(&timer, &QTimer::timeout, this, &GameView::onTick);
    timer.start();

    setFocusPolicy(Qt::StrongFocus); // working keyboard
    setFixedSize(19 * 32, 19 * 32);

    connect(&game, &Game::levelChanged, this, &GameView::levelChanged);
    connect(&game, &Game::doPlayerDeathBox, this, &GameView::onPlayerDeathBox);
    connect(&game, &Game::doLevelChoiceBox, this, &GameView::onLevelChoiceBox);

    Audio::preloadAll();
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
    static QPixmap bg(":/tiles/background.png");
    if (!bg.isNull()) painter.drawPixmap(rect(), bg);
    else painter.fillRect(rect(), Qt::black);

    game.render(&painter);
}

void GameView::keyPressEvent(QKeyEvent* event) { game.handleKeyPress(static_cast<Qt::Key>(event->key())); }

void GameView::keyReleaseEvent(QKeyEvent* event) { game.handleKeyRelease(static_cast<Qt::Key>(event->key())); }

void GameView::pauseGame() {
    game.setPaused(true);
    QMessageBox msg(parentWidget());
    msg.setIcon(QMessageBox::NoIcon);
    msg.setWindowIcon(QIcon(":/icon/pause.png"));
    msg.setWindowTitle("Пауза");
    msg.setText("Гра поставлена на паузу");
    msg.setStandardButtons(QMessageBox::Ok);
    if (QAbstractButton* ok = msg.button(QMessageBox::Ok)) ok->setText("Продовжити");
    msg.exec();
    game.setPaused(false);
}

void GameView::restartLevel() { game.restart(); }
Game* GameView::getGame() { return &game; }

void GameView::onPlayerDeathBox() {
    QMessageBox msg(parentWidget());
    msg.setIcon(QMessageBox::NoIcon);
    msg.setWindowIcon(QIcon(":/icon/lose.png"));
    msg.setWindowTitle("Ви загинули");
    msg.setText("Перезапустити рівень чи вийти?");

    QAbstractButton* ab_retry = msg.addButton("Перезапустити рівень", QMessageBox::ActionRole);
    QAbstractButton* ab_exit = msg.addButton("Вийти", QMessageBox::DestructiveRole);
    if (ab_retry) ab_retry->setObjectName("ab_retry");
    if (ab_exit)  ab_exit->setObjectName("ab_exit");
    msg.setStyleSheet(msg.styleSheet());
    msg.exec();

    QAbstractButton* clicked = msg.clickedButton();
    if (clicked == ab_retry) { game.restart(); game.finishBox(); }
    else if (clicked == ab_exit) QCoreApplication::quit();
}

void GameView::onLevelChoiceBox(int levelIndex) {
    int maxLevel = game.getMaxLevel();
    bool outOfMax = (levelIndex >= maxLevel);

    QMessageBox msg(parentWidget());
    msg.setIcon(QMessageBox::NoIcon);
    msg.setWindowIcon(QIcon(":/icon/win.png"));
    msg.setWindowTitle(outOfMax
                           ? "Вітаємо! Ви пройшли гру"
                           : QString("Рівень %1 пройдено").arg(levelIndex));
    msg.setText(outOfMax
                    ? "Повернутись до початкового екрану\nчи перезапустити рівень?"
                    : "Перейти до наступного рівня?");
    if (outOfMax) Audio::play("win");

    QAbstractButton* ab_yes   = msg.addButton(outOfMax ? "Початковий екран" : "Так", QMessageBox::AcceptRole);
    QAbstractButton* ab_retry = msg.addButton("Перезапустити рівень", QMessageBox::ActionRole);
    if (ab_retry) ab_retry->setObjectName("ab_retry");
    if (ab_yes && outOfMax) ab_yes->setObjectName("ab_startScreen");
    else if (ab_yes) ab_yes->setObjectName("ab_yes");
    msg.setStyleSheet(msg.styleSheet());
    msg.exec();

    QAbstractButton* clicked = msg.clickedButton();
    if (clicked == ab_yes) {
        if (outOfMax) { game.finishBox(); emit finishGameSession(); }
        else { game.advance(); game.finishBox(); }
    }
    else if (clicked == ab_retry) { game.restart(); game.finishBox(); }
}

