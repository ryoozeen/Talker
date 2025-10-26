#include "idfind.h"
#include "ui_idfind.h"
#include "db.h"
#include "popup.h"

#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QMessageBox>
#include <QRegularExpression>

idFind::idFind(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::idFind)
{
    ui->setupUi(this);

    // 취소 버튼 클릭 시 필드 초기화 후 back 시그널 발생
    connect(ui->exitBtn, &QPushButton::clicked, this, [this]{
        clearAllFields();  // 모든 필드 초기화
        emit back();
    });

    // id찾기
    connect(ui->findBtn, &QPushButton::clicked, this, [this]{

        // 1) 폼 값 읽기
        const QString name    = ui->nameEdit->text().trimmed();
        QString email         = ui->emailEdit->text().trimmed();
        QString domain        = ui->domain->currentText().trimmed(); // 콤보박스 선택
        QString phone         = ui->phoneEdit->text().trimmed();

        // ===== 유효성 검사 (실패 시 return; 로 종료) =====

        // 이름
        if (name.isEmpty()) {
            Popup::error(this->window(), tr("이름을 입력해주세요."), tr("ID 찾기 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        if (name.size()==1) {
            Popup::error(this->window(), tr("이름은 2글자 이상이어야 합니다."), tr("ID 찾기 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        if (name.contains(QRegularExpression("\\d"))) {
            Popup::error(this->window(), tr("이름에는 숫자를 포함할 수 없습니다."), tr("ID 찾기 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        static const QRegularExpression hangulOnlyRx(R"(^[가-힣]+$)");
        if (!hangulOnlyRx.match(name).hasMatch()) {
            Popup::error(this->window(), tr("올바른 이름을 입력해주세요."), tr("ID 찾기 실패"));
            ui->nameEdit->setFocus();
            return;
        }

        // 이메일
        if (email.isEmpty()) {
            Popup::error(this->window(), tr("이메일을 입력해주세요."), tr("ID 찾기 실패"));
            ui->emailEdit->setFocus();
            return;
        }
        if (email.length() < 8) {
            Popup::error(this->window(), tr("이메일 8글자 이상 입력해주세요."), tr("ID 찾기 실패"));
            ui->emailEdit->setFocus();
            return;
        }

        QString fullEmail;
        if (domain == tr("직접 입력")) {
            fullEmail = email;
            static const QRegularExpression emailRx(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9]+\.[A-Za-z]{2,}$)");
            if (!emailRx.match(fullEmail).hasMatch()) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("ID 찾기 실패"));
                ui->emailEdit->setFocus();
                return;
            }
        } else {
            if (domain.isEmpty() || domain == tr("선택")) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("ID 찾기 실패"));
                ui->domain->setFocus();
                return;
            }
            static const QRegularExpression localOnlyRx(R"(^[A-Za-z0-9]+$)");
            if (!localOnlyRx.match(email).hasMatch()) {
                Popup::error(this->window(), tr("특수문자는 사용할 수 없습니다."), tr("ID 찾기 실패"));
                ui->emailEdit->setFocus();
                return;
            }
            fullEmail = email + "@" + domain;   // ✅ 실제 저장값
            static const QRegularExpression emailRx2(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
            if (!emailRx2.match(fullEmail).hasMatch()) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("ID 찾기 실패"));
                ui->emailEdit->setFocus();
                return;
            }
        }

        // 휴대폰: 숫자만 → 길이/010 체크 → 포맷팅
        QString digits = phone;
        digits.remove(QRegularExpression("[^0-9]"));
        if (digits != phone) ui->phoneEdit->setText(digits);

        if (digits.length() > 0 && digits.length() < 11) {
            Popup::error(this->window(), tr("핸드폰 번호 11자리를 입력해주세요."), tr("ID 찾기 실패"));
            ui->phoneEdit->setFocus(); return; }
        if (digits.length() != 11) {
            Popup::error(this->window(), tr("핸드폰 번호 11자리를 입력해주세요."), tr("ID 찾기 실패"));
            ui->phoneEdit->setFocus();
            return;
        }
        if (!digits.startsWith("010")) {
            Popup::error(this->window(), tr("잘못된 핸드폰 번호입니다."), tr("ID 찾기 실패"));
            ui->phoneEdit->setFocus();
            return;
        }

        const QString a = digits.mid(0, 3);  // 010
        const QString b = digits.mid(3, 4);  // 1234
        const QString c = digits.mid(7, 4);  // 5678
        phone = QString("%1-%2-%3").arg(a, b, c);
        ui->phoneEdit->setText(phone);

        DB db(this->window());
        const QString foundId = db.findLoginId(name, fullEmail, phone);

        if (foundId.isEmpty()) {
            Popup::error(this->window(),
                         tr("입력하신 정보와 일치하는 계정(ID)이 없습니다."),
                         tr("ID 찾기 실패"));
            return;
        }

        Popup::info(this->window(),
                    tr("등록된 ID는 '%1' 입니다.").arg(foundId),
                    tr("ID 찾기 결과"));
        clearAllFields();
        emit findBtn();

    });

}


idFind::~idFind()
{
    delete ui;
}

void idFind::clearAllFields()
{
    // 모든 입력 필드 초기화
    ui->nameEdit->clear();
    ui->emailEdit->clear();
    ui->phoneEdit->clear();

    // 도메인 콤보박스도 기본값으로 초기화 (첫 번째 항목 선택)
    if (ui->domain->count() > 0) {
        ui->domain->setCurrentIndex(0);
    }
}
