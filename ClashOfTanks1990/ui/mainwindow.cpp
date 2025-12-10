#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gameView = new GameView(this);

    setCentralWidget(gameView);
}

MainWindow::~MainWindow()
{
    delete ui;
}
