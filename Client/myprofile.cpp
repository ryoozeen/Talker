#include "myprofile.h"
#include "ui_myprofile.h"

#include "db.h"
#include "popup.h"

#include <QPixmap>
#include <QImage>

MyProfile::MyProfile(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyProfile)
{
    ui->setupUi(this);

    // 나가기 버튼
    connect(ui->backBtn, &QPushButton::clicked, this, &MyProfile::back);

    // 아바타 라벨은 기본 원형 스타일(배경 + Talker 텍스트) 사용
}

MyProfile::~MyProfile()
{
    delete ui;
}

void MyProfile::setLoginId(const QString& loginId)
{
    loginId_ = loginId.trimmed();
    reload();
}

void MyProfile::reload()
{
    if (loginId_.isEmpty()) return;

    DB db(this);

    QString name, gender, email, phone, status, avatar;
    if (!db.getUserFullProfile(loginId_, &name, &gender, &email, &phone, &status, &avatar)) {
        Popup::error(this, tr("프로필 정보를 불러오지 못했습니다."), tr("오류"));
        return;
    }

    // 상단 상태
    ui->statusEdit->setText(status);

    // 폼 채우기
    ui->idEdit->setText(loginId_);
    ui->nameEdit->setText(name);
    ui->genderEdit->setText(gender == "M" ? tr("남") : (gender == "G" ? tr("여") : gender));
    ui->emailEdit->setText(email);
    ui->phoneEdit->setText(phone);

}
