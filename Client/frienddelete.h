#ifndef FRIENDDELETE_H
#define FRIENDDELETE_H

#include <QWidget>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QScrollArea)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)

namespace Ui { class FriendDelete; }

class FriendDelete : public QWidget
{
    Q_OBJECT
public:
    explicit FriendDelete(QWidget *parent = nullptr);
    ~FriendDelete();

    // 로그인한 내 ID를 세팅하고 목록을 즉시 갱신
    void setOwnerId(const QString& ownerId);

signals:
    void back();        // 취소 버튼 → 설정 화면으로 복귀
    void deletedOne();  // 한 명 삭제됨(메인에서 친구목록 새로고침용)

private:
    void reload();  // DB에서 읽어 다시 그리기
    QWidget* makeRow(const QString& fid,
                     const QString& name,
                     const QString& status);

private:
    Ui::FriendDelete *ui;
    QString ownerId_;
    QScrollArea* area_ = nullptr;
    QVBoxLayout* listLayout_ = nullptr;
};

#endif // FRIENDDELETE_H
