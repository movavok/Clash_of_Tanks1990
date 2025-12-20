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
    connect(&game, &Game::doMessageBox, this, &GameView::onMessageBox);
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

void GameView::onMessageBox() {
    QMessageBox msg(parentWidget());
    msg.setIcon(QMessageBox::NoIcon);
    msg.setWindowIcon(QIcon(":/icon/lose.png"));
    msg.setWindowTitle("Ви загинули");
    msg.setText("Перезапустити рівень чи вийти?");
    msg.setStandardButtons(QMessageBox::Retry | QMessageBox::Yes);
    if (QAbstractButton* retry = msg.button(QMessageBox::Retry)) retry->setText("Перезапустити рівень");
    if (QAbstractButton* yes = msg.button(QMessageBox::Yes)) yes->setText("Вийти");
    QMessageBox::StandardButton choice = static_cast<QMessageBox::StandardButton>(msg.exec());
    if (choice == QMessageBox::Retry) { game.restart(); game.finishBox(); }
    else QCoreApplication::quit();
}

void GameView::onLevelChoiceBox(int levelIndex) {
    QMessageBox msg(parentWidget());
    msg.setIcon(QMessageBox::NoIcon);
    msg.setWindowIcon(QIcon(":/icon/win.png"));
    msg.setWindowTitle(levelIndex >= 3 ? "Вітаємо! Ви пройшли гру" : QString("Рівень %1 пройдено").arg(levelIndex));
    msg.setText(levelIndex >= 3 ? "Вийти або перезапустити рівень?" : "Перейти до наступного рівня?");
    if (levelIndex >= 3) Audio::play("win");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Retry);
    if (QAbstractButton* yes = msg.button(QMessageBox::Yes)) yes->setText(levelIndex >= 3 ? "Вийти" : "Так");
    if (QAbstractButton* retry = msg.button(QMessageBox::Retry)) retry->setText("Перезапустити рівень");
    QMessageBox::StandardButton choice = static_cast<QMessageBox::StandardButton>(msg.exec());
    if (choice == QMessageBox::Yes) {
        if (levelIndex >= 3) QCoreApplication::quit();
        else { game.advance(); game.finishBox(); }
    } else { game.restart(); game.finishBox(); }
}

