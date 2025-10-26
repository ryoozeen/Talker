// chatlistsetting.cpp
#include "chatlistsetting.h"
#include "ui_chatlistsetting.h"
#include <QPushButton>

ChatListSetting::ChatListSetting(QWidget* parent)
    : QWidget(parent), ui(new Ui::ChatListSetting)
{
    ui->setupUi(this);
    connect(ui->leaveBtn,  &QPushButton::clicked, this, &ChatListSetting::leaveModeRequested);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &ChatListSetting::cancelRequested);
}
ChatListSetting::~ChatListSetting(){ delete ui; }
