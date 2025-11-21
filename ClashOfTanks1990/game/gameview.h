#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QWidget>
#include <QEvent>
#include "game.h"



class GameView : public QWidget
{
    Q_OBJECT
public:
    explicit GameView(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    Game game;
};

#endif // GAMEVIEW_H
