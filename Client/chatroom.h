// chatroom.h
#ifndef CHATROOM_H
#define CHATROOM_H

#include <QWidget>
#include <QTcpSocket>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHash>

class ChatRoom : public QWidget {
    Q_OBJECT

public:
    explicit ChatRoom(QWidget *parent = nullptr);

    void setSocket(QTcpSocket* s);
    void open(qint64 roomId, const QString& me,
    const QString& peerId, const QString& peerName);
    void handleServerMessage(const QJsonObject& obj);
    qint64 currentRoomId() const { return roomId_; }

signals:
    void backRequested();
    void messageActivity(qint64 roomId, const QString& lastBody, const QDateTime& ts);

private slots:
    void onSend();
    void onReadyRead();
    void onFilePlus();             // ← 하단 + 버튼 (이미지/동영상 전송용)

private:
    // state
    qint64 roomId_ = 0;
    QString myLoginId_;
    QString peerId_;
    QString peerName_;

    // ui
    QTextBrowser* chatView_ = nullptr;
    QLineEdit*    input_    = nullptr;
    QPushButton*  sendBtn_  = nullptr;
    QPushButton*  backBtn_  = nullptr;
    QPushButton*  searchBtn_ = nullptr;   // ⬅️ 추가
    QPushButton*  settingBtn_ = nullptr;  // ⬅️ 추가
    QPushButton*  plusBtn_  = nullptr;    // ⬅️ 파일전송 자리
    QLabel*       title_    = nullptr;

    // net
    QTcpSocket* socket_ = nullptr;

    // local cache: 방별 HTML 스냅샷 (뒤로 갔다가 들어와도 유지)
    QHash<qint64, QString> htmlCache_;    // ⬅️ 추가

    void buildUi();
    void appendBubble(const QString& who, const QString& text, const QString& ts = QString());
    void loadHistory();
    void sendJson(const QJsonObject& obj);
};

#endif // CHATROOM_H
