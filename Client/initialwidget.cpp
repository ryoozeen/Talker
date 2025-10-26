#include "initialwidget.h"
#include "ui_initialwidget.h"

#include <QPushButton>

#include <QIcon>
#include <QResizeEvent>
#include <QDebug>
#include <QDir>
#include <QFile>

static const char* kLogoPath = ":/logo/logo.png";  // alias에 맞춤

InitialWidget::InitialWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::InitialWidget) {
    ui->setupUi(this);

    // 상단 중앙 로고(QPushButton: objectName=logo)
    ui->logo->setMinimumSize(200, 200);
    ui->logo->setFlat(true);                      // 버튼 테두리 제거
    ui->logo->setText(QString());                 // 버튼 텍스트 제거
    ui->logo->setCursor(Qt::PointingHandCursor);  // 손가락 커서

    updateLogo(); // 초기 1회 적용

    // QPushButton 클릭 -> 로딩 화면 전환 시그널 emit
    connect(ui->logo, &QPushButton::clicked,
            this, &InitialWidget::onLogoClicked);
}
InitialWidget::~InitialWidget(){ delete ui; }

void InitialWidget::onLogoClicked()
{
    emit startRequested(); // 로딩 화면으로 넘어가도록 시그널 발생
}

void InitialWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateLogo(); // 라벨 크기 변경 시 다시 맞춤
}

void InitialWidget::updateLogo() {
    QPixmap raw(kLogoPath);
    if (raw.isNull()) {
        qDebug() << "[InitialWidget] 로고 로드 실패:" << kLogoPath;
        return;
    }

    // QPushButton에는 Pixmap이 아니라 Icon을 사용합니다.
    ui->logo->setIcon(QIcon(raw));
    ui->logo->setIconSize(ui->logo->size());  // 버튼 크기에 맞춰 스케일
}
