#ifndef DBUTIL_H
#define DBUTIL_H

#include <QString>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QList>

// DbUtil: DB 연결 및 자주 쓰는 쿼리 헬퍼 모음
class DbUtil {
public:
    // 환경변수 기반 연결 생성(없으면 기본값):
    //   DB_HOST, DB_NAME, DB_USER, DB_PASS
    // 성공 시 true
    static bool connectFromEnv();

    // 메시지 저장: messages(room_id, sender_login_id, body)
    // 실패 시 err에 에러 문자열
    static bool insertMessage(qint64 roomId, const QString& sender, const QString& body, QString* err);

    // dm_rooms(room_id, user_a, user_b)에서 sender의 상대 로그인ID 반환
    // 없으면 빈 문자열
    static QString peerInRoom(qint64 roomId, const QString& sender);

    // 현재 활성 DB 핸들 가져오기 (값 반환)
    static QSqlDatabase getDB();

private:
    // 내부용: 연결명으로 DB 핸들 얻기
    static QSqlDatabase db();
};

#endif // DBUTIL_H
