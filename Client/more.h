#ifndef MORE_H
#define MORE_H

#include <QWidget>

namespace Ui {
class More;
}

class More : public QWidget
{
    Q_OBJECT

public:
    explicit More(QWidget *parent = nullptr);
    ~More();

private:
    Ui::More *ui;

signals:
    void infochange();      // 회원정보 변경
    void pwchange();        // PW 변경
    void infodelete();      // 회원탈퇴
    void logout();          // 로그아웃
    void back();            // 취소
};

#endif // MORE_H
