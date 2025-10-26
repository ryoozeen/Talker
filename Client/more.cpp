#include "more.h"
#include "ui_more.h"

More::More(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::More)
{
    ui->setupUi(this);

    // 회원정보 변경
    connect(ui-> infochangeBtn, &QPushButton::clicked,
            this,               &More::infochange);

    // PW 변경
    connect(ui->pwchangeBtn, &QPushButton::clicked,
            this,           &More::pwchange);


    // 회원탈퇴
    connect(ui->infodeleteBtn, &QPushButton::clicked,
            this,              &More::infodelete);

    // 로그아웃
    connect(ui->logoutBtn, &QPushButton::clicked,
            this,           &More::logout);

    // 취소
    connect(ui->exitBtn, &QPushButton::clicked,
            this,           &More::back);

}

More::~More()
{
    delete ui;
}
