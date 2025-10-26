#ifndef SETTING_H
#define SETTING_H

#include <QWidget>

namespace Ui {
class Setting;
}

class Setting : public QWidget
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = nullptr);
    ~Setting();

private:
    Ui::Setting *ui;

signals:
    void frienddelet();         // 친구 삭제
    void profile();             // 프로필
    void profilechange();       // 프로필 변경
    void back();                // 취소
    void refresh();             // 새로고침

};

#endif // SETTING_H
