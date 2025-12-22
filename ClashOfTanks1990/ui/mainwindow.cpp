#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gameView = ui->page_gameView;
    gameView->getGame()->setPaused(true);
    
    setWindowTitle("Clash Of Tanks 1990 | lvl 1");
    setWindowIcon(QIcon(":/icon/icon.png"));
    ui->l_numberLevelAmount->setText(QString::number(gameView->getGame()->getMaxLevel()));
    connect(gameView, &GameView::levelChanged, this, &MainWindow::onLevelChanged);
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &MainWindow::onPageChanged);

    connect(ui->act_pause, &QAction::triggered, gameView, &GameView::pauseGame);
    connect(ui->act_restart, &QAction::triggered, gameView, &GameView::restartLevel);
    connect(ui->act_set, &QAction::triggered, this, &MainWindow::showVolumeDialog);
    connect(ui->act_startScreen, &QAction::triggered, this, &MainWindow::toStartScreen);

    connect(ui->b_startGame, &QPushButton::clicked, this, &MainWindow::startNewGame);
    connect(ui->b_continue, &QPushButton::clicked, this, &MainWindow::continueGame);

    // stop playerTank if clicked on qmenu
    connect(ui->menu, &QMenu::aboutToShow, this, &MainWindow::stopPlayer);
    connect(ui->menu_sound, &QMenu::aboutToShow, this, &MainWindow::stopPlayer);
    connect(ui->menu, &QMenu::aboutToHide, this, &MainWindow::stopPlayer);
    connect(ui->menu_sound, &QMenu::aboutToHide, this, &MainWindow::stopPlayer);
}

void MainWindow::onLevelChanged(int level) { setWindowTitle(QString("Clash Of Tanks 1990 | lvl %1").arg(level)); }

void MainWindow::onPageChanged(int index) {
    bool inGame = (ui->stackedWidget->widget(index) == ui->page_gameView);

    ui->act_pause->setEnabled(inGame);
    ui->act_restart->setEnabled(inGame);

    if (inGame) {
        gameView->getGame()->setPaused(false);
        gameView->setFocus();
    } else gameView->getGame()->setPaused(true);
}

void MainWindow::openGame(bool restart) {
    if(restart) {
        gameView->getGame()->newGame();
        ui->b_continue->setEnabled(canContinue = true);
    }
    ui->stackedWidget->setCurrentWidget(ui->page_gameView);
    gameView->getGame()->setPaused(false);
    gameView->setFocus();
}

void MainWindow::startNewGame() { openGame(true); }
void MainWindow::continueGame() { openGame(false); }

void MainWindow::toStartScreen() { ui->stackedWidget->setCurrentWidget(ui->page_startScreen); }

void MainWindow::stopPlayer() { gameView->getGame()->resetPlayerControls(); }

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showVolumeDialog() {
    QDialog dlg(this);
    gameView->getGame()->resetPlayerControls();
    dlg.setWindowTitle("Гучність звуку");
    dlg.setWindowIcon(QIcon(":/icon/volume.png"));
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QHBoxLayout* row = new QHBoxLayout();
    QLabel* l_volume = new QLabel("Гучність:", &dlg);
    QSlider* slider = new QSlider(Qt::Horizontal, &dlg);
    slider->setRange(0, 100);
    slider->setValue(static_cast<int>(Audio::getMasterVolume() * 100.0));
    l_percent = new QLabel(QString::number(slider->value()) + '%', &dlg);
    row->addWidget(l_volume);
    row->addWidget(slider);
    row->addWidget(l_percent);
    layout->addLayout(row);

    connect(slider, &QSlider::valueChanged, this, &MainWindow::volumeValueChanged);

    dlg.setLayout(layout);
    dlg.exec();
}

void MainWindow::volumeValueChanged(int volume) {
    Audio::setMasterVolume(volume / 100.0);
    l_percent->setText(QString::number(volume) + '%');
}
