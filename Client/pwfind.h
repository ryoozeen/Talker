#ifndef PWFIND_H
#define PWFIND_H

#include <QWidget>

namespace Ui {
class pwFind;
}

class pwFind : public QWidget
{
    Q_OBJECT

public:
    explicit pwFind(QWidget *parent = nullptr);
    ~pwFind();

private:
    Ui::pwFind *ui;
    QString verifiedLoginId;   // ✅ 검증 완료된 계정 ID 보관
    void clearAllFields();    // ✅ 모든 입력 필드 초기화 함수 추가


signals:
    void pwResetRequested(const QString& loginId);  // ✅ 추가
    void back();
    void findBtn();      // pw찾기
};

#endif // PWFIND_H
