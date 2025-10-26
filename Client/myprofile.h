#ifndef MYPROFILE_H
#define MYPROFILE_H

#include <QWidget>
#include <QString>

namespace Ui { class MyProfile; }

class MyProfile : public QWidget
{
    Q_OBJECT
public:
    explicit MyProfile(QWidget *parent = nullptr);
    ~MyProfile();

    void setLoginId(const QString& loginId); // 외부(MainWindow)에서 현재 로그인 ID 전달

signals:
    void back(); // 나가기

private:
    void reload();

private:
    Ui::MyProfile *ui;
    QString loginId_;
};

#endif // MYPROFILE_H
