#include "sign.h"
#include "ui_sign.h"
#include "db.h"
#include "popup.h"

#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QMessageBox>


Sign::Sign(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Sign)
{
    ui->setupUi(this);

    // 취소 버튼 클릭 시 필드 초기화 후 back 시그널 발생
    connect(ui->exitBtn, &QPushButton::clicked, this, [this]{
        clearAllFields();  // 모든 필드 초기화
        emit back();
    });

    // ID 중복체크 유효성 검사
    auto idCheckBtn = this->findChild<QPushButton*>("idcheckBtn");
    if (idCheckBtn) {
        connect(idCheckBtn, &QPushButton::clicked, this, [this] {
            auto idEdit = this->findChild<QLineEdit*>("idEdit");
            if (idEdit) {
                const QString loginId = idEdit->text().trimmed();

                // 빈칸
                if (loginId.isEmpty())
                {
                    Popup::error(this->window(), tr("ID를 입력해주세요."),
                                 tr("회원가입 실패"));
                    ui->idEdit->setFocus();
                    return;
                }

                // 영문만
                static const QRegularExpression idSpecialCharRx(R"([^A-Za-z0-9])");
                if (idSpecialCharRx.match(loginId).hasMatch())
                {
                    Popup::error(this->window(), tr("영문, 숫자만 입력해주세요."),
                                 tr("회원가입 실패"));
                    ui->idEdit->setFocus();
                    return;
                }

                // 8자리 이상
                if (loginId.length() < 8)
                {
                    Popup::error(this->window(), tr("8자리 이상 입력해주세요."),
                                 tr("회원가입 실패"));
                    ui->idEdit->setFocus();
                    return;
                }

                // db 연동
                DB db(this->window());
                db.checkIdDuplicate(loginId);
            }
        });
    }


    // 이메일 중복체크 유효성검사
    auto emailCheckBtn = this->findChild<QPushButton*>("emailcheckBtn");
    if (emailCheckBtn) {
        connect(emailCheckBtn, &QPushButton::clicked, this, [this] {
            auto emailEdit = this->findChild<QLineEdit*>("emailEdit");
            if (emailEdit) {
                QString email = emailEdit->text().trimmed();
                QString domain        = ui->domain->currentText().trimmed(); // 콤보박스 선택

                // 이메일
                if (email.isEmpty())
                {
                    Popup::error(this->window(), tr("이메일을 입력해주세요."),
                                 tr("회원가입 실패"));
                    ui->emailEdit->setFocus();
                    return;
                }
                if (email.length() < 8) {
                    Popup::error(this->window(), tr("이메일 8글자 이상 입력해주세요."),
                                 tr("회원가입 실패"));
                    ui->emailEdit->setFocus();
                    return;
                }

                QString fullEmail;
                if (domain == tr("직접 입력")) {
                    fullEmail = email;
                    static const QRegularExpression emailRx(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9]+\.[A-Za-z]{2,}$)");
                    if (!emailRx.match(fullEmail).hasMatch()) { Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("회원가입 실패")); ui->emailEdit->setFocus(); return; }
                } else {
                    if (domain.isEmpty() || domain == tr("선택")) { Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("회원가입 실패")); ui->domain->setFocus(); return; }
                    static const QRegularExpression localOnlyRx(R"(^[A-Za-z0-9]+$)");
                    if (!localOnlyRx.match(email).hasMatch()) { Popup::error(this->window(), tr("특수문자는 사용할 수 없습니다."), tr("회원가입 실패")); ui->emailEdit->setFocus(); return; }
                    fullEmail = email + "@" + domain;   // ✅ 실제 저장값
                    static const QRegularExpression emailRx2(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
                    if (!emailRx2.match(fullEmail).hasMatch()) { Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."), tr("회원가입 실패")); ui->emailEdit->setFocus(); return; }
                }

                DB db(this->window());
                db.checkEmailDuplicate(fullEmail);
            }
        });
    }

    // 비밀번호는 가려서 입력
    ui->pwEdit->setEchoMode(QLineEdit::Password);
    ui->pwcheckEdit->setEchoMode(QLineEdit::Password);

    // 휴대폰 입력: 숫자/점/하이픈만 허용
    auto phoneRx = QRegularExpression(QStringLiteral("^[0-9\\-\\.]*$"));
    ui->phoneEdit->setValidator(new QRegularExpressionValidator(phoneRx, this));

    // 취소 버튼 → back 시그널 (그대로)
    connect(ui->exitBtn, &QPushButton::clicked, this, &Sign::back);

    // 가입하기 버튼 → (1) 폼검사 (2) DB 저장 (3) 성공 시 join 시그널
    connect(ui->joinBtn, &QPushButton::clicked, this, [this]{

        // 1) 폼 값 읽기
        const QString name    = ui->nameEdit->text().trimmed();
        const QString loginId = ui->idEdit->text().trimmed();
        const QString gender  = ui->manBtn->isChecked() ? "M" : (ui->girlBtn->isChecked() ? "G" : "");
        const QString pw      = ui->pwEdit->text();
        const QString pwc     = ui->pwcheckEdit->text();
        QString email         = ui->emailEdit->text().trimmed();
        QString domain        = ui->domain->currentText().trimmed(); // 콤보박스 선택
        QString phone         = ui->phoneEdit->text().trimmed();

        // ===== 유효성 검사 (실패 시 return; 로 종료) =====

        // 이름
        if (name.isEmpty()) {
            Popup::error(this->window(), tr("이름을 입력해주세요."),
                         tr("회원가입 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        if (name.size()==1) {
            Popup::error(this->window(), tr("이름은 2글자 이상이어야 합니다."),
                         tr("회원가입 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        if (name.contains(QRegularExpression("\\d"))) {
            Popup::error(this->window(), tr("이름에는 숫자를 포함할 수 없습니다."),
                         tr("회원가입 실패"));
            ui->nameEdit->setFocus();
            return;
        }
        static const QRegularExpression hangulOnlyRx(R"(^[가-힣]+$)");
        if (!hangulOnlyRx.match(name).hasMatch()) {
            Popup::error(this->window(), tr("올바른 이름을 입력해주세요."),
                         tr("회원가입 실패"));
            ui->nameEdit->setFocus();
            return;
        }

        // 성별
        if (!ui->manBtn->isChecked() && !ui->girlBtn->isChecked()) {
            Popup::error(this->window(), tr("성별 체크를 해주세요."),
                         tr("회원가입 실패"));
            ui->manBtn->setFocus();
            return;
        }

        // ID
        if (loginId.isEmpty()) {
            Popup::error(this->window(), tr("ID를 입력해주세요."),
                         tr("회원가입 실패"));
            ui->idEdit->setFocus();
            return;
        }
        static const QRegularExpression idSpecialCharRx(R"([^A-Za-z0-9])");
        if (idSpecialCharRx.match(loginId).hasMatch()) {
            Popup::error(this->window(), tr("영문, 숫자만 입력해주세요."),
                         tr("회원가입 실패"));
            ui->idEdit->setFocus();
            return;
        }
        if (loginId.length() < 8) {
            Popup::error(this->window(), tr("8자리 이상 입력해주세요."),
                         tr("회원가입 실패"));
            ui->idEdit->setFocus();
            return;
        }

        // PW
        if (pw.isEmpty()) {
            Popup::error(this->window(), tr("PW를 입력해주세요."),
                         tr("회원가입 실패"));
            ui->pwEdit->setFocus();
            return;
        }
        static const QRegularExpression pwRx(R"((?=.*[A-Za-z])(?=.*\d).{8,})");
        if (!pwRx.match(pw).hasMatch()) {
            Popup::error(this->window(), tr("영문 + 숫자 조합 8자리 이상 입력해주세요."),
                         tr("회원가입 실패"));
            ui->pwEdit->setFocus();
            return;
        }
        if (pwc.isEmpty() || pw != pwc) {
            Popup::error(this->window(), tr("PW가 일치하지 않습니다."),
                         tr("회원가입 실패"));
            ui->pwcheckEdit->setFocus();
            return;
        }

        // 이메일
        if (email.isEmpty()) {
            Popup::error(this->window(), tr("이메일을 입력해주세요."),
                         tr("회원가입 실패"));
            ui->emailEdit->setFocus();
            return;
        }
        if (email.length() < 8) {
            Popup::error(this->window(), tr("이메일 8글자 이상 입력해주세요."),
                         tr("회원가입 실패"));
            ui->emailEdit->setFocus();
            return;
        }

        QString fullEmail;
        if (domain == tr("직접 입력")) {
            fullEmail = email;
            static const QRegularExpression emailRx(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9]+\.[A-Za-z]{2,}$)");
            if (!emailRx.match(fullEmail).hasMatch()) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."),
                             tr("회원가입 실패"));
                ui->emailEdit->setFocus();
                return;
            }
        } else {
            if (domain.isEmpty() || domain == tr("선택")) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."),
                             tr("회원가입 실패"));
                ui->domain->setFocus();
                return;
            }
            static const QRegularExpression localOnlyRx(R"(^[A-Za-z0-9]+$)");
            if (!localOnlyRx.match(email).hasMatch()) {
                Popup::error(this->window(), tr("특수문자는 사용할 수 없습니다."),
                             tr("회원가입 실패"));
                ui->emailEdit->setFocus();
                return;
            }
            fullEmail = email + "@" + domain;   // ✅ 실제 저장값
            static const QRegularExpression emailRx2(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
            if (!emailRx2.match(fullEmail).hasMatch()) {
                Popup::error(this->window(), tr("올바른 이메일 형식이 아닙니다."),
                             tr("회원가입 실패"));
                ui->emailEdit->setFocus();
                return;
            }
        }

        // 휴대폰: 숫자만 → 길이/010 체크 → 포맷팅
        QString digits = phone;
        digits.remove(QRegularExpression("[^0-9]"));
        if (digits != phone) ui->phoneEdit->setText(digits);

        if (digits.length() > 0 && digits.length() < 11) {
            Popup::error(this->window(), tr("핸드폰 번호 11자리를 입력해주세요."),
                         tr("회원가입 실패"));
            ui->phoneEdit->setFocus(); return; }
        if (digits.length() != 11) {
            Popup::error(this->window(), tr("핸드폰 번호 11자리를 입력해주세요."),
                         tr("회원가입 실패"));
            ui->phoneEdit->setFocus();
            return;
        }
        if (!digits.startsWith("010")) {
            Popup::error(this->window(), tr("잘못된 핸드폰 번호입니다."),
                         tr("회원가입 실패"));
            ui->phoneEdit->setFocus();
            return;
        }

        const QString a = digits.mid(0, 3);  // 010
        const QString b = digits.mid(3, 4);  // 1234
        const QString c = digits.mid(7, 4);  // 5678
        phone = QString("%1-%2-%3").arg(a, b, c);
        ui->phoneEdit->setText(phone);

        // ===== 여기까지 오면 통과 → DB 저장 =====
        DB db(this->window());
        if (db.insertUser(loginId, name, gender, fullEmail, phone, pw)) {  // ✅ fullEmail 사용
            Popup::success(this->window(), tr("회원가입이 완료되었습니다."),
                         tr("회원가입 성공"));
            clearAllFields();
            emit join();
        }
        // 불필요한 'return true;' 절대 넣지 않기
    });
}

Sign::~Sign()
{
    delete ui;
}

void Sign::clearAllFields()
{
    // 모든 입력 필드 초기화
    ui->idEdit->clear();
    ui->nameEdit->clear();
    ui->pwEdit->clear();
    ui->pwcheckEdit->clear();
    ui->emailEdit->clear();
    ui->phoneEdit->clear();

    // 도메인 콤보박스도 기본값으로 초기화 (첫 번째 항목 선택)
    if (ui->domain->count() > 0) {
        ui->domain->setCurrentIndex(0);
    }

    // 성별 라디오버튼 초기화
    ui->manBtn->setAutoExclusive(false);   // 그룹 해제
    ui->girlBtn->setAutoExclusive(false);

    ui->manBtn->setChecked(false);         // 체크 해제
    ui->girlBtn->setChecked(false);

    ui->manBtn->setAutoExclusive(true);    // 다시 그룹 활성화
    ui->girlBtn->setAutoExclusive(true);
}
