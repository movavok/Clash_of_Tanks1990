#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gameView = new GameView(this);

    setCentralWidget(gameView);
    connect(gameView, &GameView::levelChanged, this, &MainWindow::onLevelChanged);
}

void MainWindow::onLevelChanged(int level) { setWindowTitle(QString("Clash Of Tanks 1990 | lvl %1").arg(level)); }

MainWindow::~MainWindow()
{
    delete ui;
}
