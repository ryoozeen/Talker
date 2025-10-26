#ifndef FIND_H
#define FIND_H

#include <QWidget>

namespace Ui {
class Find;
}

class Find : public QWidget
{
    Q_OBJECT

public:
    explicit Find(QWidget *parent = nullptr);
    ~Find();

private:
    Ui::Find *ui;

signals:
    void back();        // 취소
    void idFind();      // id찾기
    void pwFind();      // pw찾기

};



#endif // FIND_H
