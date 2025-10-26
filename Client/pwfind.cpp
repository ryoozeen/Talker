#include "pwfind.h"
#include "ui_pwfind.h"
#include "db.h"
#include "popup.h"
#include "pwreset.h"

#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpression>

pwFind::pwFind(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::pwFind)
{
    ui->setupUi(this);

    // 취소 버튼 클릭 시 필드 초기화 후 back 시그널 발생
    connect(ui->exitBtn, &QPushButton::clicked, this, [this]{
        clearAllFields();  // 모든 필드 초기화
        emit back();
    });

    // pw찾기
    connect(ui->findBtn, &QPushButton::clicked, this, [this]{

        // 1) 폼 값 읽기
        const QString loginId = ui->idEdit->text().trimmed();
        const QString name    = ui->nameEdit->text().trimmed();
        QString email         = ui->emailEdit->text().trimmed();
        QString domain        = ui->domain->currentText().trimmed(); // 콤보박스 선택
        QString phone         = ui->phoneEdit->text().trimmed();

        // ===== 유효성 검사 (실패 시 return; 로 종료) =====

        // ID
        if (loginId.isEmpty()) {
            Popup::error(this->window(), tr("ID를 입력해주세요."), tr("PW 찾기 실패"));
            ui->idEdit->setFocus();
            return;
        }
        static const QRegularExpression idSpecialCharRx(R"([^A-Za-z0-9])");
        if (idSpecialCharRx.match(loginId).hasMatch()) {
            Popup::error(this->window(), tr("ID는 영문, 숫자만 입력해주세요."),
                         tr("PW 찾기 실패"));
            ui->idEdit->setFocus();
            return;
        }
        if (loginId.length() < 8) {
            Popup::error(this->window(), tr("8자리 이상 입력해주세요."),
                         tr("PW 찾기 실패"));
            ui->idEdit->setFocus();
            return;
        }

        // 이름
        if (name.isEmpty()) {
            Popup::error(this->window(), tr("이름을 입력해주세요."), tr("PW 찾기 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        if (name.size()==1) {
            Popup::error(this->window(), tr("이름은 2글자 이상이어야 합니다."), tr("PW 찾기 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        if (name.contains(QRegularExpression("\\d"))) {
            Popup::error(this->window(), tr("이름에는 숫자를 포함할 수 없습니다."), tr("PW 찾기 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        static const QRegularExpression hangulOnlyRx(R"(^[가-힣]+$)");
        if (!hangulOnlyRx.match(name).hasMatch()) {
            Popup::error(this->window(), tr("올바른 이름을 입력해주세요."), tr("PW 찾기 실패"));
            ui->nameEdit->setFocus();
            return;
        }

        // 이메일
        if (email.isEmpty()) {
            Popup::error(this->window(), tr("이메일을 입력해주세요."), tr("PW 찾기 실패"));
            ui->emailEdit->setFocus();
            return;
        }
        if (email.length() < 8) {
            Popup::error(this->window(), tr("이메일 8글자 이상 입력해주세요."), tr("PW 찾기 실패"));
            ui->emailEdit->setFocus();
            return;
        }

        QString fullEmail;
        if (domain == tr("직접 입력")) {
            fullEmail = email;
            static const QRegularExpression emailRx(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9]+\.[A-Za-z]{2,}$)");
            if (!emailRx.match(fullEmail).hasMatch()) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("PW 찾기 실패"));
                ui->emailEdit->setFocus();
                return;
            }
        } else {
            if (domain.isEmpty() || domain == tr("선택")) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("PW 찾기 실패"));
                ui->domain->setFocus();
                return;
            }
            static const QRegularExpression localOnlyRx(R"(^[A-Za-z0-9]+$)");
            if (!localOnlyRx.match(email).hasMatch()) {
                Popup::error(this->window(), tr("특수문자는 사용할 수 없습니다."), tr("PW 찾기 실패"));
                ui->emailEdit->setFocus();
                return;
            }
            fullEmail = email + "@" + domain;   // ✅ 실제 저장값
            static const QRegularExpression emailRx2(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
            if (!emailRx2.match(fullEmail).hasMatch()) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("PW 찾기 실패"));
                ui->emailEdit->setFocus();
                return;
            }
        }

        // 휴대폰: 숫자만 → 길이/010 체크 → 포맷팅
        QString digits = phone;
        digits.remove(QRegularExpression("[^0-9]"));
        if (digits != phone) ui->phoneEdit->setText(digits);

        if (digits.length() > 0 && digits.length() < 11) {
            Popup::error(this->window(), tr("핸드폰 번호 11자리를 입력해주세요."), tr("PW 찾기 실패"));
            ui->phoneEdit->setFocus(); return; }
        if (digits.length() != 11) {
            Popup::error(this->window(), tr("핸드폰 번호 11자리를 입력해주세요."), tr("PW 찾기 실패"));
            ui->phoneEdit->setFocus();
            return;
        }
        if (!digits.startsWith("010")) {
            Popup::error(this->window(), tr("잘못된 핸드폰 번호입니다."), tr("PW 찾기 실패"));
            ui->phoneEdit->setFocus();
            return;
        }

        const QString a = digits.mid(0, 3);  // 010
        const QString b = digits.mid(3, 4);  // 1234
        const QString c = digits.mid(7, 4);  // 5678
        phone = QString("%1-%2-%3").arg(a, b, c);
        ui->phoneEdit->setText(phone);

        // ===== 여기까지 오면 통과 → DB 검증 =====
        DB db(this->window());

        // 1) loginId + name + fullEmail + phone 으로 계정 존재/일치 검증
        const bool ok = db.verifyAccountForPwReset(
            loginId,   // 사용자가 입력한 ID
            name,
            fullEmail, // 위에서 조립한 email
            phone      // "010-1234-5678" 형태
            );

        if (!ok) {
            Popup::error(this->window(),
                         tr("입력하신 정보와 일치하는 계정을 찾지 못했습니다."),
                         tr("PW 찾기 실패"));
            return;
        }

        // DB 검증 성공 직후에
        clearAllFields();
        emit pwResetRequested(loginId);   // ✅ 여기서 화면 전환하지 않음(팝업/다이얼로그 생성 X)
    });
}

pwFind::~pwFind()
{
    delete ui;
}

void pwFind::clearAllFields()
{
    // 모든 입력 필드 초기화
    ui->idEdit->clear();
    ui->nameEdit->clear();
    ui->emailEdit->clear();
    ui->phoneEdit->clear();

    // 도메인 콤보박스도 기본값으로 초기화 (첫 번째 항목 선택)
    if (ui->domain->count() > 0) {
        ui->domain->setCurrentIndex(0);
    }
}
