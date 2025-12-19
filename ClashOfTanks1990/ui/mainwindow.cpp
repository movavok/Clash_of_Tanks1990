#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    gameView = new GameView(this);

    setCentralWidget(gameView);
    setWindowTitle("Clash Of Tanks 1990 | lvl 1");
    setWindowIcon(QIcon(":/icon/icon.png"));
    connect(gameView, &GameView::levelChanged, this, &MainWindow::onLevelChanged);

    connect(ui->act_pause, &QAction::triggered, gameView, &GameView::pauseGame);
    connect(ui->act_restart, &QAction::triggered, gameView, &GameView::restartLevel);
    connect(ui->act_set, &QAction::triggered, this, &MainWindow::showVolumeDialog);
}

void MainWindow::onLevelChanged(int level) { setWindowTitle(QString("Clash Of Tanks 1990 | lvl %1").arg(level)); }

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showVolumeDialog() {
    QDialog dlg(this);
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
