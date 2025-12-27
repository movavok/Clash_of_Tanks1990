#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QIcon>
#include <QKeyCombination>
#include <QApplication>
#include <cstdlib>
#include <ctime>
#include "../game/gameview.h"
#include "../systems/audio.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    GameView* gameView;
    QLabel* l_percent;
    QSlider* slider;

    bool canContinue = false;

protected:
    bool event(QEvent*) override;

private slots:
    void onLevelChanged(int);
    void onPageChanged(int);
    void onFinishedGame();

    void openGame(bool);
    void startNewGame();
    void continueGame();
    void toStartScreen();

    void showVolumeDialog();
    void volumeValueChanged(int);

    void stopPlayer();
};
#endif // MAINWINDOW_H
