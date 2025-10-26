#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Setting)
{
    ui->setupUi(this);

    // 친구 삭제
    connect(ui->frienddeletBtn, &QPushButton::clicked,
            this,           &Setting::frienddelet);

    // 프로필
    connect(ui->profileBtn, &QPushButton::clicked,
            this,           &Setting::profile);

    // 프로필 변경
    connect(ui->profilechangeBtn, &QPushButton::clicked,
            this,           &Setting::profilechange);

    // 취소
    connect(ui->exitBtn, &QPushButton::clicked,
            this,           &Setting::back);

    // 새로고침
    connect(ui->refreshBtn, &QPushButton::clicked,
            this,            &Setting::refresh);
}

Setting::~Setting()
{
    delete ui;
}
