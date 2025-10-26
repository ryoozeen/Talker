#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#pragma once
#include <QWidget>
#include <QVariantMap>
#include <QString>
#include <QEvent>        // ⬅️ 추가
#include <QMouseEvent>   // ⬅️ 추가

// C -> S
#define C2S_STATUS_NOTIFY "C2S_STATUS_NOTIFY"   // 내 상태메세지 바뀌었음을 서버에 알림

// S -> C
#define S2C_FRIEND_STATUS_CHANGED "S2C_FRIEND_STATUS_CHANGED" // 친구 상태메세지 변경 push

namespace Ui {
class friendList;
}

class friendList : public QWidget
{
    Q_OBJECT

public:
    explicit friendList(QWidget *parent = nullptr);
    ~friendList();

    void appendFriendCard(const QString& friendId, const QString& name, const QString& status);
    void refreshFromDb(const QString& myId);

    // ⬇️ 추가: 설정 화면에서 '친구삭제' 눌렀을 때 토글할 삭제 모드
    void setDeleteMode(bool on) { deleteMode_ = on; }   // 한 줄짜리 간단 세터

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;  // ⬅️ 추가

private:
    Ui::friendList *ui;
    bool deleteMode_ = false; // 삭제 모드 플래그
    QString ownerId_;         // refreshFromDb 호출 시 내 ID 기억
    QWidget* makeProfileCard(const QString& titleLine, const QString& subLine,
                             bool emphasize = false, QWidget* parent = nullptr);

signals:
    void setting();        // 세팅
    void serch();          // 검색
    void chat();           // 채팅
    void more();           // 더보기
    void friendlist();     // 친구
    void friendClicked(const QString& friendId, const QString& friendName); // 친구클릭 > 채팅

};

#endif // FRIENDLIST_H
