#include "dbutil.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QProcessEnvironment>

// Qt 독립 연결명을 정해둠 (중복 연결 생성 방지)
static QString kConnName = "chatserver_mysql";

// 외부에 노출되는 DB 핸들 getter
QSqlDatabase DbUtil::getDB() {
    return db();   // private db() 호출해 반환
}

// 내부용: 연결명으로 DB 핸들을 얻는다.
// - connectFromEnv()가 먼저 호출되어 연결이 생성되어 있어야 정상 반환
QSqlDatabase DbUtil::db() {
    return QSqlDatabase::database(kConnName);
}

// 환경변수에서 DB 접속정보 읽어 연결 생성
// - 이미 같은 이름의 연결이 존재하면 true 반환(중복 생성 안 함)
bool DbUtil::connectFromEnv() {
    if (QSqlDatabase::contains(kConnName)) return true;

    // 시스템 환경변수 읽기
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString host = env.value("DB_HOST","127.0.0.1");
    const QString name = env.value("DB_NAME","talker");
    const QString user = env.value("DB_USER","root");
    const QString pass = env.value("DB_PASS","");

    // MySQL 드라이버(QMYSQL)로 연결 추가
    QSqlDatabase d = QSqlDatabase::addDatabase("QMYSQL", kConnName);
    d.setHostName(host);
    d.setDatabaseName(name);
    d.setUserName(user);
    d.setPassword(pass);

    if (!d.open()) {
        qWarning() << "DB open failed:" << d.lastError().text();
        return false;
    }
    return true;
}

// 메시지 저장: messages(room_id, sender_login_id, body)
// - created_at 등은 DB DEFAULT CURRENT_TIMESTAMP 컬럼으로 처리 가정
bool DbUtil::insertMessage(qint64 roomId, const QString& sender, const QString& body, QString* err) {
    QSqlQuery q(db());
    q.prepare("INSERT INTO messages(room_id, sender_login_id, body) VALUES (?,?,?)");
    q.addBindValue(roomId);
    q.addBindValue(sender);
    q.addBindValue(body);
    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }
    return true;
}

// 같은 방의 상대 로그인ID 조회
// - dm_rooms(user_a, user_b) 테이블에서 sender가 user_a면 user_b, 반대면 user_a
QString DbUtil::peerInRoom(qint64 roomId, const QString& sender) {
    QSqlQuery q(db());
    q.prepare(R"(
        SELECT CASE WHEN user_a = ? THEN user_b ELSE user_a END AS peer
        FROM dm_rooms
        WHERE room_id = ? LIMIT 1
    )");
    q.addBindValue(sender);
    q.addBindValue(roomId);
    if (!q.exec()) return QString();
    if (q.next()) return q.value(0).toString();
    return QString();
}
