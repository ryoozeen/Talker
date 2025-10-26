#ifndef DB_H
#define DB_H

#pragma once
#include <QString>
#include <QPixmap>
#include <QMainWindow>
#include <QSqlDatabase>
#include <QVariantMap>

QT_BEGIN_NAMESPACE
namespace Ui { class DB; }
QT_END_NAMESPACE

class DB : public QMainWindow {
    Q_OBJECT

public:
    explicit DB(QWidget *parent = nullptr);
    ~DB();

    bool connectTodb();                                  // 내부 전용 연결 함수

    // 회원가입 저장 (UI 의존 없음: 순수 값만 받음)
    bool insertUser(const QString& loginId,
                    const QString& name,
                    const QString& gender,     // "M" or "G"
                    const QString& email,
                    const QString& phone,
                    const QString& plainPw);

    bool checkIdDuplicate(const QString& loginId);      // 로그인 중복체크
    bool checkEmailDuplicate(const QString& email);     // 이메일 중복체크

    // 로그인 DB
    bool verifyLogin(const QString& loginId, const QString& plainPw);

    // ID 찾기 DB
    QString findLoginId(const QString& name,
                        const QString& email,
                        const QString& phone);

    // PW 찾기 DB
    bool verifyAccountForPwReset(const QString& loginId,
                                 const QString& name,
                                 const QString& email,
                                 const QString& phone);

    // PW 변경: 새 비밀번호로 salt 재발급 + 해시 갱신
    bool updatePassword(const QString& loginId, const QString& newPlainPw);

    bool userExistsById(const QString& loginId);
    bool getUserProfileById(const QString& loginId, QString* nameOut, QString* statusOut);
    // 친구추가
    bool addFriend(const QString& ownerId, const QString& friendId, QString* errorOut = nullptr);
    bool listFriends(const QString& ownerId, QList<QVariantMap>* out);

    // 친구삭제
    bool removeFriend(const QString& ownerId, const QString& friendId, QString* errorOut = nullptr);

    // 나의 프로필 화면
    bool getUserFullProfile(const QString& loginId,
                            QString* nameOut,
                            QString* genderOut,
                            QString* emailOut,
                            QString* phoneOut,
                            QString* statusOut,
                            QString* avatarUrlOut);

    // 프로필 변경
    bool upsertStatusMessage(const QString& loginId,
                             const QString& status,
                             QString* errorOut = nullptr);

    // 방 보장(없으면 생성, 있으면 접속정보 업데이트)
    // mySocket: 내 QTcpSocket::socketDescriptor(), myIp: peerAddress().toString()

    static bool ensureConnection(QString* err=nullptr);  // ★ 추가
    static QSqlDatabase handle();                        // ★ 추가

    bool ensureDmRoom(const QString& me, const QString& peerId,
                      int mySocket, const QString& myIp,
                      qint64* outRoomId, QString* err);

    // 채팅방 목록(나 기준)
    QList<QVariantMap> listDmRooms(const QString& me, QString* err);

    // 채팅방 나가기
    bool deleteDmRoom(qint64 roomId, const QString& me, QString* err=nullptr);

    // 메시지 저장 (서버에서 호출 권장)
    bool insertMessage(qint64 roomId, const QString& senderLoginId,
                       const QString& body, QString* err);

    // 특정 방의 최근 메시지 로드
    QList<QVariantMap> listMessages(qint64 roomId, int limit, int offset, QString* err);

    // 입장 문구에 “이름(아이디)”
    QString getUserName(const QString& loginId, QString* err = nullptr);

    QList<QVariantMap> fetchChatList(const QString& me, QString* err = nullptr);

    // “방 나가기”를 전역 삭제 → 내 쪽만 숨김/초기화 로 변경
    bool leaveDmRoom(qint64 roomId, const QString& me, QString* err=nullptr);   // 내 쪽만 숨김+초기화
    bool unhideDmRoom(qint64 roomId, const QString& me, QString* err=nullptr);  // 내 쪽 숨김 해제
    bool getClearedAt(qint64 roomId, const QString& me, QDateTime* out, QString* err=nullptr);


private:
    Ui::DB *ui{nullptr};
    QSqlDatabase db;
    // 사용자별 상태 테이블 추가
    static bool ensureUserRoomStateTable(QSqlDatabase& h, QString* err=nullptr);

};

#endif // DB_H




