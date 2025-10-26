#include "chatserver.h"
#include <QJsonDocument>    // JSON 직렬화/역직렬화
#include <QJsonObject>
#include <QDateTime>        // 타임스탬프 생성
#include <QHostAddress>
#include <QSqlQuery>        // DB 쿼리
#include <QSqlError>
#include <QDebug>
#include "dbutil.h"         // DB 연결/헬퍼

// 생성자: 특별한 초기화 없음 (listen은 main에서 수행)
ChatServer::ChatServer(QObject *parent) : QTcpServer(parent) {}

// 새 클라이언트 연결 수락 시 호출 (accept 완료 상태)
// - 들어온 소켓 디스크립터를 QTcpSocket에 연결하고, readyRead/disconnected 시그널 연결
void ChatServer::incomingConnection(qintptr handle) {
    auto s = new QTcpSocket(this);           // 부모를 this로 지정: 서버가 파괴되면 소켓도 정리
    s->setSocketDescriptor(handle);          // OS 소켓 디스크립터 적용
    connect(s, &QTcpSocket::readyRead, this, &ChatServer::onReadyRead);
    connect(s, &QTcpSocket::disconnected, this, &ChatServer::onDisconnected);
    qInfo() << "client connected:" << s->peerAddress().toString() << s->peerPort();
}

// 소켓에서 데이터가 들어올 때 호출
// - 본 서버는 "라인 단위(JSON 한 줄)" 프로토콜을 사용
void ChatServer::onReadyRead() {
    auto s = qobject_cast<QTcpSocket*>(sender());
    if (!s) return;
    while (s->canReadLine()) {                      // 개행(\n)까지 읽을 수 있을 때
        const auto line = s->readLine().trimmed();  // 개행 제거
        if (!line.isEmpty()) handleLine(s, line);   // 비어있지 않으면 처리
    }
}

// 소켓이 끊길 때 호출
// - 매핑에서 제거하고 deleteLater로 안전한 파괴 예약
void ChatServer::onDisconnected() {
    auto s = qobject_cast<QTcpSocket*>(sender());
    if (!s) return;
    const auto id = idBySocket_.take(s);   // 소켓→ID 매핑 제거 (존재하면 반환)
    if (!id.isEmpty()) socketById_.remove(id); // ID→소켓 매핑도 제거
    s->deleteLater();
    qInfo() << "client disconnected" << id;
}

// ─────────────────────────────────────────────────────────────
// 같은 방의 두 사용자 모두에게 obj를 전송
// - dm_rooms(room_id, user_a, user_b) 테이블을 사용한다고 가정
// - 해당 방의 user_a/user_b를 조회하여 현재 접속 중이면 write()
// ─────────────────────────────────────────────────────────────
void ChatServer::sendToRoomBoth(qint64 roomId, const QJsonObject& obj)
{
    QSqlQuery q(DbUtil::getDB());
    q.prepare("SELECT user_a, user_b FROM dm_rooms WHERE room_id=? LIMIT 1");
    q.addBindValue(roomId);
    if (!q.exec() || !q.next()) return;    // 방이 없거나 쿼리 실패 시 무시

    const QString a = q.value(0).toString();
    const QString b = q.value(1).toString();

    // id→소켓 조회 후 전송
    auto sendTo = [&](const QString& id) {
        if (QTcpSocket* s = socketById_.value(id, nullptr)) {
            s->write(QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n");
        }
    };
    sendTo(a);
    sendTo(b);
}

// ─────────────────────────────────────────────────────────────
// 상태메세지 변경 알림 처리
// 입력 JSON 예:
//   { "type": "status_notify", "status": "공부중" }
// 처리 흐름:
//   1) 나(me) 확인 (소켓→ID 매핑)
//   2) (선택) user_profile에 UPSERT로 반영
//   3) friends 테이블을 조회해서 나를 구독한 사람(친구) 찾기
//   4) 각 친구 소켓에 S2C_FRIEND_STATUS_CHANGED 푸시
// ─────────────────────────────────────────────────────────────
void ChatServer::handleStatusNotify(QTcpSocket* s, const QJsonObject& obj)
{
    const QString me = idBySocket_.value(s);   // 소켓→ID
    if (me.isEmpty()) return;

    // 상태메세지는 최대 길이 제한(예: 255)으로 자름
    const QString status = obj.value("status").toString().left(255);

    // (선택) 서버에서도 DB에 반영해두면, 클라와 정합성 유지에 유리
    {
        QSqlQuery up(DbUtil::getDB());
        up.prepare(R"SQL(
            INSERT INTO user_profile(login_id, status_message)
            VALUES (?, ?)
            ON DUPLICATE KEY UPDATE
              status_message = VALUES(status_message),
              updated_at     = CURRENT_TIMESTAMP
        )SQL");
        up.addBindValue(me);
        up.addBindValue(status);
        if (!up.exec()) {
            qWarning() << "[status upsert fail]" << up.lastError().text();
        }
    }

    // 나를 구독하는(=친구 목록에 나를 가진) 대상 찾기
    // friends(owner_id, friend_id) 스키마 가정:
    // - owner_id가 me인 행: 내가 등록한 친구들
    // - friend_id가 me인 행: 나를 친구로 등록한 사람들
    QSqlQuery q(DbUtil::getDB());
    q.prepare(R"SQL(
        SELECT owner_id AS target
        FROM friends
        WHERE friend_id = ?
        UNION
        SELECT friend_id AS target
        FROM friends
        WHERE owner_id = ?   -- 양방향 미등록 케이스 보완
    )SQL");
    q.addBindValue(me);
    q.addBindValue(me);
    if (!q.exec()) {
        qWarning() << "[status notify] query fail:" << q.lastError().text();
        return;
    }

    // 푸시 페이로드 구성 (compact JSON + 개행)
    QJsonObject push{
                     {"type",     "S2C_FRIEND_STATUS_CHANGED"},
                     {"login_id", me},
                     {"status",   status},
                     };
    const QByteArray bytes = QJsonDocument(push).toJson(QJsonDocument::Compact) + "\n";

    // 해당 대상들에게 전송 (현재 접속 중인 소켓만)
    int cnt = 0;
    while (q.next()) {
        const QString target = q.value(0).toString();
        if (QTcpSocket* t = socketById_.value(target, nullptr)) {
            t->write(bytes);
            ++cnt;
        }
    }
    qInfo() << "[status_notify]" << me << "-> broadcast:" << cnt;
}

// ─────────────────────────────────────────────────────────────
// 1줄(JSON) 메시지 처리
// 지원 타입:
//   - "login": 로그인ID 등록(소켓↔ID 매핑)
//   - "status_notify" / "C2S_STATUS_NOTIFY": 상태메세지 알림
//   - "chat": DM 채팅 메시지 저장 + 상대에게 푸시
//   - "enter": 입장 알림 저장 + 양쪽 브로드캐스트
// 공통 규칙:
//   - 보낸 사람(sender)는 클라이언트 JSON을 신뢰하지 않고,
//     서버가 기억한 소켓→ID 매핑에서 가져온다.
// ─────────────────────────────────────────────────────────────
void ChatServer::handleLine(QTcpSocket* s, const QByteArray& line) {
    const auto obj = QJsonDocument::fromJson(line).object();
    const QString type = obj.value("type").toString();

    // 1) 로그인 처리: {"type":"login","login_id":"alice"}
    if (type == "login") {
        const QString loginId = obj.value("login_id").toString().trimmed();
        if (loginId.isEmpty()) {
            QJsonObject out{{"type","error"},{"reason","empty_login_id"}};
            s->write(QJsonDocument(out).toJson(QJsonDocument::Compact) + "\n");
            s->disconnectFromHost();
            return;
        }

        // 서버 측에서도 존재 검증(최소한의 검증)
        QSqlQuery q(DbUtil::getDB());
        q.prepare("SELECT 1 FROM users WHERE login_id=? LIMIT 1");
        q.addBindValue(loginId);
        if (!q.exec() || !q.next()) {
            QJsonObject out{{"type","error"},{"reason","invalid_login"}};
            s->write(QJsonDocument(out).toJson(QJsonDocument::Compact) + "\n");
            s->disconnectFromHost();
            return;
        }

        idBySocket_[s] = loginId;       // 소켓→ID
        socketById_[loginId] = s;       // ID→소켓
        qInfo() << "login:" << loginId;
        return;
    }

    // 2) 상태메세지 변경 알림
    if (type == "status_notify" || type == "C2S_STATUS_NOTIFY") {
        handleStatusNotify(s, obj);
        return;
    }

    // 3) 공통 추출: room_id, sender, text
    //   - sender는 반드시 서버 매핑에서 가져온 값 사용(보안/신뢰성)
    const qint64 roomId = static_cast<long long>(obj.value("room_id").toDouble());
    const QString sender = idBySocket_.value(s);          // 신뢰 가능한 보낸 사람
    const QString text   = obj.value("text").toString();  // 본문

    // 3-1) 채팅 메시지: DB 저장 후 상대에게만 푸시
    if (type == "chat") {
        // (1) DB 저장
        QString err;
        if (!DbUtil::insertMessage(roomId, sender, text, &err)) {
            qWarning() << "insertMessage failed:" << err;
        }

        // (2) 상대에게 전달 (보낸 사람 화면은 클라가 이미 표시)
        const QString peer = DbUtil::peerInRoom(roomId, sender); // 같은 방의 상대 ID
        if (!peer.isEmpty()) {
            if (auto ps = socketById_.value(peer, nullptr)) {
                QJsonObject out{
                    {"type","chat"},
                    {"room_id", static_cast<double>(roomId)},
                    {"sender", sender},
                    {"text", text},
                    {"ts", QDateTime::currentDateTime().toString(Qt::ISODate)}
                };
                ps->write(QJsonDocument(out).toJson(QJsonDocument::Compact) + "\n");
            }
        }
        return;
    }
    // 3-2) 입장 알림: 저장 + 양쪽 브로드캐스트
    else if (type == "enter") {
        // 요구사항: "입장하셨습니다" 같은 시스템 메시지도 저장 권장
        QString err;
        if (!DbUtil::insertMessage(roomId, sender, text, &err)) {
            qWarning() << "insert enter failed:" << err;
        }

        // 양쪽 모두에게 브로드캐스트(보낸 사람 화면에도 즉시 반영)
        QJsonObject out{
            {"type","enter"},
            {"room_id", static_cast<double>(roomId)},
            {"sender", sender},
            {"text", text},
            {"ts", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
        sendToRoomBoth(roomId, out);
        return;
    }

    // 3-3) 파일 전송: base64 데이터(소용량) 즉시 전달
    else if (type == "file") {
        // 필수 필드: filename, mime, data(base64)
        const QString filename = obj.value("filename").toString();
        const QString mime     = obj.value("mime").toString();
        const QString b64      = obj.value("data").toString();
        if (filename.isEmpty() || b64.isEmpty()) {
            QJsonObject out{{"type","error"},{"reason","bad_file_payload"}};
            s->write(QJsonDocument(out).toJson(QJsonDocument::Compact) + "\n");
            return;
        }

        const QByteArray bytes = QByteArray::fromBase64(b64.toLatin1());
        // 간단한 크기 제한: 5 MB
        constexpr qint64 kMax = 5 * 1024 * 1024;
        if (bytes.size() <= 0 || bytes.size() > kMax) {
            QJsonObject out{{"type","error"},{"reason","file_too_large"}};
            s->write(QJsonDocument(out).toJson(QJsonDocument::Compact) + "\n");
            return;
        }

        // 수신자에게 그대로 전달(서버는 현재 보관하지 않음)
        QJsonObject out{{"type","file"},
                        {"room_id", static_cast<double>(roomId)},
                        {"sender", sender},
                        {"filename", filename},
                        {"mime", mime},
                        {"data", b64},
                        {"ts", QDateTime::currentDateTime().toString(Qt::ISODate)}};

        const QString peer = DbUtil::peerInRoom(roomId, sender);
        if (!peer.isEmpty()) {
            if (auto ps = socketById_.value(peer, nullptr)) {
                ps->write(QJsonDocument(out).toJson(QJsonDocument::Compact) + "\n");
            }
        }
        return;
    }
}
