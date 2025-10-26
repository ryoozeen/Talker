#ifndef SERCH_H
#define SERCH_H

#include <QWidget>
#include <QString>

namespace Ui {
class Serch;
}

class Serch : public QWidget
{
    Q_OBJECT

public:
    explicit Serch(QWidget *parent = nullptr);
    ~Serch();

    // ✅ 로그인한 내 ID를 넘겨받아 저장
    void setMyId(const QString& myId) { myId_ = myId.trimmed(); }

private:
    Ui::Serch *ui;
    QString myId_;   // ✅ 내 ID 저장
    void clearAllFields();    // ✅ 모든 입력 필드 초기화 함수 추가

signals:
    void back();            // 취소
    void searchRequested(const QString& friendLoginId);  // ✅ 검색 요청
    void candidateReady(const QString& friendId, const QString& name, const QString& status);
};

#endif // SERCH_H
