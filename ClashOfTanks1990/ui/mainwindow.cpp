#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gameView = ui->page_gameView;
    gameView->getGame()->setPaused(true);

    setWindowTitle("Clash Of Tanks 1990");
    setWindowIcon(QIcon(":/icon/icon.png"));
    ui->l_numberLevelAmount->setText("<b>" + QString::number(gameView->getGame()->getMaxLevel()) + "</b>");
    ui->b_aboutBoosts->setToolTip(
        "Підсилювачі:\n"
        "• Швидкість: +50% до руху на 8 с.\n"
        "• Перезарядка: швидша стрільба та більші кулі на 8 с.\n"
        "• Щит: поглинає один влучний постріл.\n\n"
        "Підбираються як гравцем, так і ворогами."
        );

    connect(gameView, &GameView::levelChanged, this, &MainWindow::onLevelChanged);
    connect(gameView, &GameView::finishGameSession, this, &MainWindow::onFinishedGame);
    connect(gameView, &GameView::framesDrawed, this, &MainWindow::fpsUpdated);
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

void MainWindow::fpsUpdated(int fps) {
    unsigned short maxFPS = ui->sb_maxFPS->value();
    ui->l_fpsNumber->setText(fps < maxFPS ? QString::number(fps) : QString::number(maxFPS));
}

void MainWindow::onPageChanged(int index) {
    bool inGame = (ui->stackedWidget->widget(index) == ui->page_gameView);

    ui->act_pause->setEnabled(inGame);
    ui->act_restart->setEnabled(inGame);

    if (inGame) {
        gameView->getGame()->setPaused(false);
        gameView->setFocus();
    } else gameView->getGame()->setPaused(true);
}

bool MainWindow::event(QEvent* event) {
    if (event->type() == QEvent::WindowActivate &&
        gameView->getGame()->isPaused() &&
        ui->stackedWidget->currentWidget() == ui->page_gameView) gameView->getGame()->setPaused(false);
    else if (event->type() == QEvent::WindowDeactivate &&
             !gameView->getGame()->isPaused() &&
             ui->stackedWidget->currentWidget() == ui->page_gameView) gameView->getGame()->setPaused(true);

    return QMainWindow::event(event);
}

void MainWindow::openGame(bool restart) {
    if(restart) {
        gameView->getGame()->newGame();
        ui->b_continue->setEnabled(canContinue = true);
    }
    ui->stackedWidget->setCurrentWidget(ui->page_gameView);
    gameView->getGame()->setMovementScheme(ui->cb_movement->currentIndex(), ui->kse_shoot->keySequence()[0].key());
    gameView->setMaxFPS(ui->sb_maxFPS->value());
}

void MainWindow::startNewGame() { openGame(true); }
void MainWindow::continueGame() { openGame(false); }

void MainWindow::toStartScreen() { ui->stackedWidget->setCurrentWidget(ui->page_startScreen); }

void MainWindow::onFinishedGame() {
    canContinue = false;
    ui->b_continue->setEnabled(canContinue);
    setWindowTitle("Clash Of Tanks 1990");
    toStartScreen();
}

void MainWindow::stopPlayer() { gameView->getGame()->resetPlayerControls(); }

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showVolumeDialog() {
    QDialog dialog(this);
    gameView->getGame()->resetPlayerControls();
    dialog.setWindowTitle("Гучність звуку");
    dialog.setWindowIcon(QIcon(":/icon/volume.png"));
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QHBoxLayout* row = new QHBoxLayout();
    QLabel* l_volume = new QLabel("Гучність:", &dialog);
    slider = new QSlider(Qt::Horizontal, &dialog);
    slider->setRange(0, 100);
    slider->setValue(static_cast<int>(Audio::getMasterVolume() * 100.0));
    slider->setFixedSize(100, 12);
    l_percent = new QLabel(QString::number(slider->value()) + '%', &dialog);
    row->addWidget(l_volume);
    row->addWidget(slider);
    row->addWidget(l_percent);
    layout->addLayout(row);

    connect(slider, &QSlider::valueChanged, this, &MainWindow::volumeValueChanged);

    dialog.setLayout(layout);
    dialog.exec();
}

void MainWindow::volumeValueChanged(int volume) {
    Audio::setMasterVolume(volume / 100.0);
    l_percent->setText(QString::number(volume) + '%');

    int gradient = 100 + volume;
    slider->setStyleSheet(QString(R"(
        QDialog QSlider::sub-page:horizontal {
            border: 1px solid #163816;
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:0,
                stop:0 rgb(40,%1,40),
                stop:1 rgb(80,%1,80)
            );
        }
    )").arg(gradient));
}
