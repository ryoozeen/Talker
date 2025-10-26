#ifndef PROFILECHANGE_H
#define PROFILECHANGE_H

#include <QWidget>

namespace Ui { class ProfileChange; }

class ProfileChange : public QWidget {
    Q_OBJECT
public:
    explicit ProfileChange(QWidget *parent = nullptr);
    ~ProfileChange();

    // MainWindow에서 현재 사용자 이름/상태를 주입해서 표시
    void setInitial(const QString& meName, const QString& currentStatus);

signals:
    void cancelRequested();                 // 취소 버튼
    void applyRequested(const QString&);    // 변경 버튼(입력 텍스트 전달)

private:
    Ui::ProfileChange *ui;
};

#endif // PROFILECHANGE_H
