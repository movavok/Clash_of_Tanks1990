#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
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
};

#endif // GAMEVIEW_H
