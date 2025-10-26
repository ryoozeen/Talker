// chatlistitem.cpp
#include "chatlistitem.h"
#include "ui_chatlistitem.h"
#include <QPushButton>
#include <QLabel>

ChatListItem::ChatListItem(QWidget* parent)
    : QWidget(parent), ui(new Ui::ChatListItem)
{
    ui->setupUi(this);
    connect(ui->exitBtn, &QPushButton::clicked, this, [this]{
        emit exitClicked(roomId_, ui->nameLabel->text());
    });

}

ChatListItem::~ChatListItem(){ delete ui; }

void ChatListItem::setData(qint64 roomId, const QString& name, const QString& preview, const QString& date){
    roomId_ = roomId;
    ui->nameLabel->setText(name);
    ui->previewLabel->setText(preview);
    ui->dateLabel->setText(date);
}
void ChatListItem::setLeaveMode(bool on){ ui->exitBtn->setVisible(on); }
