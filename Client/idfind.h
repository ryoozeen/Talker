#ifndef IDFIND_H
#define IDFIND_H

#include <QWidget>

namespace Ui {
class idFind;
}

class idFind : public QWidget
{
    Q_OBJECT

public:
    explicit idFind(QWidget *parent = nullptr);
    ~idFind();

private:
    Ui::idFind *ui;
    void clearAllFields();    // ✅ 모든 입력 필드 초기화 함수 추가

signals:
    void back();        // 취소
    void findBtn();      // id찾기

};

#endif // IDFIND_H
