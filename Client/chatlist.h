#ifndef CHATLIST_H
#define CHATLIST_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>

class ChatList : public QWidget {
    Q_OBJECT
public:
    explicit ChatList(QWidget *parent = nullptr);
    void reload(const QString& me);     // DB에서 방 목록 로드
    void setLeaveMode(bool on);  // 나가기 모드 토글

public slots:
    void onMessageActivity(qint64 roomId, const QString& lastBody, const QDateTime& ts);

signals:
    void backRequested();
    void openRoomRequested(qint64 roomId, const QString& peerId, const QString& peerName);
    void openSettingRequested();    // 상단 톱니바퀴 눌렀을 때 설정 화면으로
    void exitClicked(qint64 roomId, const QString& displayName);

    // 하단 탭 (친구/채팅/더보기) — 친구목록 화면과 동일하게
    void friendTabRequested();
    void chatTabRequested();
    void moreTabRequested();

private:
    void bumpRoom(qint64 roomId, const QString& lastBody, const QDateTime& ts);

    bool leaveMode_ = false;     // 나가기 모드 on/off
    QString me_;                 // reload(me)로 받은 내 로그인ID 캐시
    QListWidget* roomList_ = nullptr;
    QPushButton* backBtn_ = nullptr;

    // 하단 탭 버튼
    QPushButton *tabFriend_ = nullptr, *tabChat_ = nullptr, *tabMore_ = nullptr;

    void buildUi();
    void populate(const QList<QVariantMap>& rows);
    void showError(const QString& msg);

    // ⬇️ 한 행(카드) UI 생성
    QWidget* makeRow(const QString& displayName,
                     const QString& lastBody,
                     const QDateTime& lastTs,
                     QWidget* parent = nullptr);
};

#endif // CHATLIST_H
