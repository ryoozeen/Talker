#include "pwreset.h"
#include "ui_pwreset.h"
#include "db.h"
#include "popup.h"

#include <QRegularExpression>
#include <QPushButton>
#include <QLineEdit>

pwReset::pwReset(QWidget *parent)
    : QWidget(parent), ui(new Ui::pwReset) {
    ui->setupUi(this);

    // 버튼 objectName이 changeBtn, 입력창이 pwEdit/pwCheckEdit 라고 가정
    connect(ui->resetBtn, &QPushButton::clicked, this, [this]{
        if (loginId_.isEmpty()) {
            Popup::error(this->window(), tr("내부 오류: 로그인 ID 없음"), tr("PW 변경 실패"));
            return;
        }
        const QString pw  = ui->pwEdit->text();
        const QString pwc = ui->pwcheckEdit->text();

        // --- PW 유효성 검사 ---
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

        // --- DB 갱신 ---
        DB db(this->window());
        if (!db.updatePassword(loginId_, pw)) {
            // 내부에서 에러 팝업 처리됨
            return;
        }

        Popup::success(this->window(),
                       tr("비밀번호가 변경되었습니다."),
                       tr("완료"));

        // ✅ 로그인 화면으로 돌려보내기 위한 신호
        emit done();
    });

    // 비밀번호 입력칸은 가려서 표시
    ui->pwEdit->setEchoMode(QLineEdit::Password);
    ui->pwcheckEdit->setEchoMode(QLineEdit::Password);
}

pwReset::~pwReset() {            // ✅ 반드시 정의
    delete ui;
}

void pwReset::setLoginId(const QString& id) {
    loginId_ = id.trimmed();
    ui->pwEdit->clear();
    ui->pwcheckEdit->clear();
    ui->pwEdit->setFocus();
}
