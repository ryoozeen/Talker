#include "find.h"
#include "ui_find.h"

Find::Find(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Find)
{
    ui->setupUi(this);

    // 취소
    connect(ui->exitBtn, &QPushButton::clicked,
            this,           &Find::back);

    // id찾기
    connect(ui->idBtn, &QPushButton::clicked,
            this,           &Find::idFind);

    // pw찾기
    connect(ui->pwBtn, &QPushButton::clicked,
            this,           &Find::pwFind);
}

Find::~Find()
{
    delete ui;
}
