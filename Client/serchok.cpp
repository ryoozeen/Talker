#include "serchok.h"
#include "ui_serchok.h"
#include "popup.h"

SerchOk::SerchOk(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SerchOk)
{
    ui->setupUi(this);

        connect(ui->exitBtn, &QPushButton::clicked, this, &SerchOk::back);

        connect(ui->addfriendBtn, &QPushButton::clicked, this, [this]{
            if (friendId_.isEmpty()) {
                Popup::warning(this, tr("유효한 친구 데이터가 없습니다."), tr("추가 실패"));
                return;
            }
            emit addFriendRequested(friendId_);
        });
    }

    void SerchOk::setProfile(const QString& friendId, const QString& name, const QString& status) {
        friendId_ = friendId;
        ui->nameLabel->setText(name);
        ui->statusLabel->setText(status.isEmpty() ? tr("상태메시지 없음") : status);
        // GroupBox 제목, 아이콘 등은 디자이너에서
    }

SerchOk::~SerchOk()
{
    delete ui;
}
