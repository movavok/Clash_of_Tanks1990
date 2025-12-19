#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QKeyEvent>
#include "../systems/audio.h"
#include <QMessageBox>
#include "game.h"

class GameView : public QWidget
{
    Q_OBJECT
public:
    explicit GameView(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;

private:
    Game game;
    QTimer timer;
    long long lastTime = 0;

private slots:
    void onTick();

public slots:
    void pauseGame();
    void restartLevel();
    Game* getGame();

signals:
    void levelChanged(int);
};

#endif // GAMEVIEW_H
