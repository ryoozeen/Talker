#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>   // TCP 서버 베이스 클래스
#include <QTcpSocket>   // 개별 클라이언트 소켓
#include <QHash>        // 로그인ID <-> 소켓 매핑용
#include <QJsonObject>  // JSON 메시지 포맷
#include <QJsonDocument> // ⬅️ (중요) inline 함수에서 사용되므로 헤더에서 포함 권장

// ChatServer: QTcpServer 상속, 채팅 프로토콜 처리
class ChatServer : public QTcpServer {
    Q_OBJECT
public:
    // 생성자: QObject 부모 전달 가능
    explicit ChatServer(QObject* parent=nullptr);

protected:
    // 새 연결이 수락될 때(accept) OS 소켓 디스크립터(handle)가 들어옴
    // 여기서 QTcpSocket을 생성하고 시그널 연결까지 수행
    void incomingConnection(qintptr handle) override;

private slots:
    // 소켓에서 읽을 데이터가 도착했을 때 호출 (line 단위로 처리)
    void onReadyRead();

    // 소켓이 끊겼을 때(에러/정상 종료 포함) 호출
    void onDisconnected();

private:
    // ───────── 소켓/ID 매핑 ─────────
    // 소켓 → 로그인ID (누가 보냈는지 확인)
    QHash<QTcpSocket*, QString> idBySocket_;
    // 로그인ID → 소켓 (특정 사용자에게 푸시 보낼 때)
    QHash<QString, QTcpSocket*> socketById_;

    // 1줄(JSON) 수신 처리: type별로 분기 (login/chat/enter/status_notify 등)
    void handleLine(QTcpSocket* s, const QByteArray& line);

    // 같은 방의 두 사용자 모두에게 JSON 메시지 전송 (DM 양방향 브로드캐스트)
    void sendToRoomBoth(qint64 roomId, const QJsonObject& obj);

    // 상태메세지 변경 알림 처리: DB upsert + 친구들에게 푸시
    void handleStatusNotify(QTcpSocket* s, const QJsonObject& obj);

    // (편의) 특정 로그인ID에게 JSON 푸시
    inline void pushTo(const QString& loginId, const QJsonObject& obj) {
        if (auto t = socketById_.value(loginId, nullptr)) {
            // Compact JSON + 개행(\n) — onReadyRead()가 라인 기준 처리하므로 필수
            t->write(QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n");
        }
    }
};

#endif // CHATSERVER_H
