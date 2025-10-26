#include "serch.h"
#include "ui_serch.h"
#include "serchok.h"
#include "popup.h"
#include "db.h"

#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QMessageBox>

Serch::Serch(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Serch)
{
    ui->setupUi(this);

    // 취소 버튼 클릭 시 필드 초기화 후 back 시그널 발생
    connect(ui->exitBtn, &QPushButton::clicked, this, [this]{
        clearAllFields();  // 모든 필드 초기화
        emit back();
    });

    connect(ui->serchBtn, &QPushButton::clicked, this, [this]{
        const QString fid = ui->friendidEdit->text().trimmed();
        if (fid.isEmpty()) {
            Popup::warning(this, tr("친구 ID를 입력하세요."), tr("검색"));
            ui->friendidEdit->setFocus();
            return;
        }
        clearAllFields();

        // ✅ 내 ID는 검색 불가
        if (!myId_.isEmpty() && fid.compare(myId_, Qt::CaseInsensitive) == 0) {
            Popup::warning(this, tr("본인 ID는 검색할 수 없습니다."), tr("검색"));
            ui->friendidEdit->setFocus();
            return;
        }

        DB db(this->window());
        if (!db.userExistsById(fid)) {
            Popup::error(this, tr("존재하지 않는 ID입니다."), tr("검색 실패"));
            ui->friendidEdit->setFocus();
            return;
        }

        QString name, status;
        if (!db.getUserProfileById(fid, &name, &status)) {
            Popup::error(this, tr("프로필을 불러오지 못했습니다."), tr("검색 실패"));
            return;
        }

        emit candidateReady(fid, name, status);
    });
}

Serch::~Serch()
{
    delete ui;
}

void Serch::clearAllFields()
{
    // 모든 입력 필드 초기화
    ui->friendidEdit->clear();
}
