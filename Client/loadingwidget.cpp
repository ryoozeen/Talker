#include "loadingwidget.h"
#include "ui_loadingwidget.h"
#include <QTimer>

LoadingWidget::LoadingWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoadingWidget), timer(new QTimer(this)), value(0) {
    ui->setupUi(this);

    connect(timer, &QTimer::timeout, this, &LoadingWidget::tick);
    timer->setInterval(20);
}

LoadingWidget::~LoadingWidget(){ delete ui; }

void LoadingWidget::start(){
    value = 0;
    ui->progressBar->setValue(value);
    timer->start();
}

void LoadingWidget::tick(){
    value += 1;
    if (value > 100) value = 100;
    ui->progressBar->setValue(value);
    if (value >= 100){
        timer->stop();
        emit finished();
    }
}
