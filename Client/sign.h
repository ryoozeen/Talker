#ifndef SIGN_H
#define SIGN_H

#include <QWidget>

namespace Ui {
class Sign;
}

class Sign : public QWidget
{
    Q_OBJECT

public:
    explicit Sign(QWidget *parent = nullptr);
    ~Sign();

private:
    Ui::Sign *ui;
    bool validateForm();              // 모든 유효성 검사 + 휴대폰 정규화
    void clearAllFields();    // ✅ 모든 입력 필드 초기화 함수 추가

signals:
    void back();      // "취소"
    void join();      // "가입하기"
};

#endif // SIGN_H
