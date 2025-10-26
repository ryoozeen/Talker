#include "db.h"
#include "ui_db.h"
#include "popup.h"

#include <QMessageBox>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>
#include <QVariant>   // (보통 이미 있지만 안전하게)
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

DB::DB(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DB)
{
    ui->setupUi(this);
}

DB::~DB()
{
    delete ui;
}

static QString makeSalt16() {
    // 8바이트 랜덤 → hex 16자 (스키마 salt CHAR(16)에 정확히 맞춤)
    QByteArray raw(8, Qt::Uninitialized);
    for (int i = 0; i < raw.size(); ++i) raw[i] = char(QRandomGenerator::global()->generate() & 0xFF);
    return QString::fromLatin1(raw.toHex()); // 길이 16
}

static QString sha256Hex(const QByteArray& data) {
    auto hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    return QString::fromLatin1(hash.toHex()); // 64 hex chars
}

bool DB::connectTodb() {
    // 환경변수 기반 단일 연결(kConn)로 일원화
    QString err;
    if (!ensureConnection(&err)) {
        qWarning() << "[DB] 연결 실패:" << err;
        Popup::error(this,
                     tr("DB 연결 실패: %1").arg(err),
                     tr("DB 오류"),
                     420);
        return false;
    }
    db = handle();
    return db.isValid() && db.isOpen();
}

// ID 유효성 검사
bool DB::checkIdDuplicate(const QString& loginId) {
    if (!connectTodb()) return false;

    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM users WHERE login_id = :id");
    q.bindValue(":id", loginId);

    if (!q.exec()) {
        Popup::error(this,
                     tr("ID 중복 검사 실패"),  // 오류 상세
                     tr("DB 오류"));
        return false;
    }
    q.next();
    const int cnt = q.value(0).toInt();
    if (cnt > 0) {
        Popup::error(this->window(),
                     tr("이미 등록된 ID입니다."),
                     tr("실패"));
        return false;
    }
    Popup::success(this,
                    tr("사용 가능한 ID입니다."),
                    tr("성공"));
    return true;
}

// 이메일 유효성 검사
bool DB::checkEmailDuplicate(const QString& email) {
    const QString em = email.trimmed();
    if (em.isEmpty()) {

        QMessageBox::warning(this, "확인", "이메일을 입력하세요.");
        return false;
    }

    if (!connectTodb()) return false;

    QSqlQuery q(db);
    q.prepare(R"SQL(
        SELECT EXISTS(SELECT 1 FROM users WHERE email = :email) AS ex
    )SQL");
    q.bindValue(":email", em);

    if (!q.exec()) {
        Popup::error(this,
                     tr("이메일 중복 검사 실패."),
                     tr("오류"));
        return false;
    }
    if (!q.next()) {
        Popup::error(this,
                     tr("이메일 중복 검사 결과를 가져오지 못했습니다."),
                     tr("오류"));
        return false;
    }

    const bool exists = q.value(0).toInt() == 1;
    if (exists) {
        Popup::error(this,
                     tr("이미 등록된 이메일입니다."),                            // 오류 상세
                     tr("실패"));                                            // 코드
        return false;
    } else {
        Popup::success(this,
                     tr("사용 가능한 이메일입니다."),                            // 오류 상세
                     tr("성공"));                                            // 코드
        return true;
    }
}

bool DB::insertUser(const QString& loginId,
                    const QString& name,
                    const QString& gender,   // "M" or "G"
                    const QString& email,
                    const QString& phone,
                    const QString& plainPw)
{
    if (!connectTodb()) return false;

    // 2) salt & hash
    const QString salt = makeSalt16(); // CHAR(16)에 정확히 맞춤
    const QString passHash = sha256Hex((salt + plainPw).toUtf8()); // 64 hex

    // 3) INSERT
    QSqlQuery q(db);
    q.prepare(R"SQL(
        INSERT INTO users (login_id, name, gender, email, phone, salt, pass_hash)
        VALUES (:id, :name, :gender, :email, :phone, :salt, :hash)
    )SQL");
    q.bindValue(":id", loginId);
    q.bindValue(":name", name);
    q.bindValue(":gender", gender);   // "M" or "G"
    q.bindValue(":email", email);
    q.bindValue(":phone", phone);
    q.bindValue(":salt",  salt);
    q.bindValue(":hash",  passHash);

    if (!q.exec()) {
        const auto err = q.lastError();
        const QString txt = err.text();

        // MySQL: Duplicate entry ... for key 'users.login_id' 같은 형식
        if (err.nativeErrorCode() == "1062" || txt.contains("Duplicate entry")) {
            QString which;
            if (txt.contains("login_id")) which = tr("ID");
            else if (txt.contains("email")) which = tr("이메일");
            else if (txt.contains("phone")) which = tr("핸드폰");
            else which = tr("ID/이메일/핸드폰");

            Popup::error(this->window(),
                         tr("%1이(가) 이미 사용 중입니다.").arg(which),
                         tr("중복"));
            return false;
        }

        Popup::error(this->window(),
                     tr("회원가입 실패 : %1").arg(txt),
                     tr("DB 오류"));
        return false;
    }

    return true;
}

// ID찾기 유효성 검사
QString DB::findLoginId(const QString& name,
                        const QString& email,
                        const QString& phone)
{
    if (!connectTodb()) return QString();

    QSqlQuery q(db);
    q.prepare(R"SQL(
        SELECT login_id
          FROM users
         WHERE name  = :name
           AND email = :email
           AND phone = :phone
         LIMIT 1
    )SQL");
    q.bindValue(":name",  name.trimmed());
    q.bindValue(":email", email.trimmed());
    q.bindValue(":phone", phone.trimmed());

    if (!q.exec()) {
        Popup::error(this->window(),
                     tr("ID 조회 실패: %1").arg(q.lastError().text()),
                     tr("DB 오류"));
        return QString();
    }
    if (!q.next()) {
        // 일치하는 계정 없음
        return QString();
    }
    return q.value(0).toString();
}

bool DB::verifyAccountForPwReset(const QString& loginId,
                                 const QString& name,
                                 const QString& email,
                                 const QString& phone)
{
    if (!connectTodb()) return false;

    QSqlQuery q(db);
    q.prepare(R"SQL(
        SELECT COUNT(*) FROM users
         WHERE login_id = :id
           AND name     = :name
           AND email    = :email
           AND phone    = :phone
    )SQL");
    q.bindValue(":id",    loginId.trimmed());
    q.bindValue(":name",  name.trimmed());
    q.bindValue(":email", email.trimmed());
    q.bindValue(":phone", phone.trimmed());

    if (!q.exec()) {
        Popup::error(this->window(),
                     tr("계정 확인 실패: %1").arg(q.lastError().text()),
                     tr("DB 오류"));
        return false;
    }
    q.next();
    return (q.value(0).toInt() > 0);
}

// PW 찾기 유효성 검사
bool DB::updatePassword(const QString& loginId, const QString& newPlainPw)
{
    if (!connectTodb()) return false;

    // 새 salt + 해시 생성
    const QString salt     = makeSalt16();
    const QString passHash = sha256Hex((salt + newPlainPw).toUtf8());

    QSqlQuery q(db);
    q.prepare(R"SQL(
        UPDATE users
           SET salt = :salt, pass_hash = :hash
         WHERE login_id = :id
        LIMIT 1
    )SQL");
    q.bindValue(":salt", salt);
    q.bindValue(":hash", passHash);
    q.bindValue(":id",   loginId);

    if (!q.exec()) {
        Popup::error(this->window(),
                     tr("비밀번호 변경 실패: %1").arg(q.lastError().text()),
                     tr("DB 오류"));
        return false;
    }
    if (q.numRowsAffected() != 1) {
        Popup::error(this->window(),
                     tr("비밀번호 변경 대상 계정을 찾지 못했습니다."),
                     tr("실패"));
        return false;
    }
    return true;
}

bool DB::verifyLogin(const QString& loginId, const QString& plainPw)
{
    if (!connectTodb()) return false;

    // 1) DB에서 해당 ID의 salt와 pass_hash를 가져오기
    QSqlQuery q(db);
    q.prepare(R"SQL(
        SELECT salt, pass_hash
        FROM users
        WHERE login_id = :id
        LIMIT 1
    )SQL");
    q.bindValue(":id", loginId.trimmed());

    if (!q.exec()) {
        Popup::error(this->window(),
                     tr("로그인 검증 실패: %1").arg(q.lastError().text()),
                     tr("DB 오류"));
        return false;
    }

    // 2) 해당 ID가 존재하지 않는 경우
    if (!q.next()) {
        return false; // 계정이 존재하지 않음
    }

    // 3) DB에서 가져온 salt와 hash
    const QString dbSalt = q.value(0).toString();
    const QString dbHash = q.value(1).toString();

    // 4) 입력받은 비밀번호를 같은 방식으로 해시화
    const QString inputHash = sha256Hex((dbSalt + plainPw).toUtf8());

    // 5) 해시 비교
    return (inputHash == dbHash);
}

bool DB::userExistsById(const QString& loginId) {
    if (!connectTodb()) return false;
    QSqlQuery q(db);
    q.prepare("SELECT EXISTS(SELECT 1 FROM users WHERE login_id=:id)");
    q.bindValue(":id", loginId.trimmed());
    if (!q.exec() || !q.next()) return false;
    return q.value(0).toInt() == 1;
}

bool DB::getUserProfileById(const QString& loginId,
                            QString* nameOut,
                            QString* statusOut) {
    if (!connectTodb()) return false;
    QSqlQuery q(db);
    q.prepare(R"SQL(
      SELECT u.name, p.status_message
        FROM users u
        LEFT JOIN user_profile p ON p.login_id = u.login_id
       WHERE u.login_id = :id
       LIMIT 1
    )SQL");
    q.bindValue(":id", loginId.trimmed());
    if (!q.exec() || !q.next()) return false;
    if (nameOut)   *nameOut   = q.value(0).toString();
    if (statusOut) *statusOut = q.value(1).toString(); // NULL이면 빈 문자열
    return true;
}

bool DB::addFriend(const QString& ownerId, const QString& friendId, QString* errorOut) {
    if (!connectTodb()) { if (errorOut) *errorOut = "DB 연결 실패"; return false; }

    const QString me = ownerId.trimmed();
    const QString fr = friendId.trimmed();

    if (me.isEmpty() || fr.isEmpty()) { if (errorOut) *errorOut = "owner/friend가 비어있음"; return false; }
    if (me.compare(fr, Qt::CaseInsensitive) == 0) {
        if (errorOut) *errorOut = "자기 자신은 추가할 수 없습니다.";
        return false;
    }
    // 존재 검증(선택) — 이미 검색화면에서 검증하므로 생략해도 OK
    // if (!userExistsById(fr)) { if (errorOut) *errorOut = "존재하지 않는 ID"; return false; }

    QSqlQuery q(db);
    q.prepare(R"SQL(
      INSERT INTO friends(owner_id, friend_id)
      VALUES(:o, :f)
      ON DUPLICATE KEY UPDATE created_at = created_at
    )SQL");
    q.bindValue(":o", me);
    q.bindValue(":f", fr);
    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }
    return true;
}

bool DB::listFriends(const QString& ownerId, QList<QVariantMap>* out) {
    if (!connectTodb()) return false;
    QSqlQuery q(db);
    q.prepare(R"SQL(
      SELECT u.login_id, u.name, COALESCE(p.status_message, '') AS status_message, f.created_at
        FROM friends f
        JOIN users u           ON u.login_id = f.friend_id
        LEFT JOIN user_profile p ON p.login_id = u.login_id
       WHERE f.owner_id = :me
       ORDER BY u.name
    )SQL");
    q.bindValue(":me", ownerId.trimmed());
    if (!q.exec()) return false;

    if (out) {
        while (q.next()) {
            QVariantMap row;
            row["login_id"] = q.value(0).toString();
            row["name"]     = q.value(1).toString();
            row["status"]   = q.value(2).toString();

            // 안전 변환 (헤더: #include <QDateTime>)
            const QVariant v = q.value(3);
            QDateTime dt;
            if (v.canConvert<QDateTime>()) {
                dt = v.toDateTime();
            } else {
                dt = QDateTime::fromString(v.toString(), "yyyy-MM-dd HH:mm:ss");
                dt.setTimeSpec(Qt::LocalTime);
            }
            row["created"]  = dt; // 또는 row["created"] = v.toString();

            out->push_back(row);
        }
    }
    return true;
}

bool DB::removeFriend(const QString& ownerId, const QString& friendId, QString* errorOut)
{
    if (!connectTodb()) {
        if (errorOut) *errorOut = "DB 연결 실패";
        return false;
    }

    QSqlQuery q(db);
    q.prepare(R"SQL(
        DELETE FROM friends
         WHERE owner_id = :o AND friend_id = :f
        LIMIT 1
    )SQL");
    q.bindValue(":o", ownerId.trimmed());
    q.bindValue(":f", friendId.trimmed());

    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }
    // numRowsAffected()가 0이면 이미 없는 관계
    return (q.numRowsAffected() >= 1);
}

bool DB::getUserFullProfile(const QString& loginId,
                            QString* nameOut,
                            QString* genderOut,
                            QString* emailOut,
                            QString* phoneOut,
                            QString* statusOut,
                            QString* avatarUrlOut) {
    if (!connectTodb()) return false;
    QSqlQuery q(db);
    q.prepare(R"SQL(
        SELECT u.name,
               u.gender,
               u.email,
               u.phone,
               COALESCE(p.status_message, ''),
               COALESCE(p.avatar_url, '')
          FROM users u
          LEFT JOIN user_profile p ON p.login_id = u.login_id
         WHERE u.login_id = :id
         LIMIT 1
    )SQL");
    q.bindValue(":id", loginId.trimmed());
    if (!q.exec() || !q.next()) return false;

    if (nameOut)      *nameOut      = q.value(0).toString();
    if (genderOut)    *genderOut    = q.value(1).toString();
    if (emailOut)     *emailOut     = q.value(2).toString();
    if (phoneOut)     *phoneOut     = q.value(3).toString();
    if (statusOut)    *statusOut    = q.value(4).toString();
    if (avatarUrlOut) *avatarUrlOut = q.value(5).toString();
    return true;
}

// 채팅방
bool DB::ensureDmRoom(const QString& me, const QString& peer,
                      int mySock, const QString& myIp,
                      qint64* outRoomId, QString* err)
{
    if (!ensureConnection(err)) return false;
    QSqlDatabase db = handle();
    QString a = (me < peer) ? me : peer;
    QString b = (me < peer) ? peer : me;

    // 1) 방 존재 보장 (INSERT IGNORE + SELECT)
    QSqlQuery ins(db);
    ins.prepare(R"(INSERT IGNORE INTO dm_rooms(user_a,user_b) VALUES(?,?))");
    ins.addBindValue(a);
    ins.addBindValue(b);
    if (!ins.exec()) { if (err) *err = ins.lastError().text(); /* 계속 진행 */ }

    // 2) 내 소켓/IP 업데이트(옵션)
    QSqlQuery up(db);
    up.prepare(R"(
        UPDATE dm_rooms
           SET a_socket = IF(user_a=?, ?, a_socket),
               b_socket = IF(user_b=?, ?, b_socket),
               a_ip     = IF(user_a=?, ?, a_ip),
               b_ip     = IF(user_b=?, ?, b_ip)
         WHERE user_a=? AND user_b=? )");
    up.addBindValue(me); up.addBindValue(mySock);
    up.addBindValue(me); up.addBindValue(mySock);
    up.addBindValue(me); up.addBindValue(myIp);
    up.addBindValue(me); up.addBindValue(myIp);
    up.addBindValue(a);  up.addBindValue(b);
    up.exec(); //

    // 3) room_id 회수
    QSqlQuery sel(db);
    sel.prepare("SELECT room_id FROM dm_rooms WHERE user_a=? AND user_b=?");
    sel.addBindValue(a); sel.addBindValue(b);
    if (!sel.exec() || !sel.next()) { if (err) *err = sel.lastError().text(); return false; }

    if (outRoomId) *outRoomId = sel.value(0).toLongLong();
    return true;
}

QList<QVariantMap> DB::listDmRooms(const QString& me, QString* err)
{
    QList<QVariantMap> out;
    if (!ensureConnection(err)) return out;
    QSqlDatabase h = handle();
    QSqlQuery q(h);
    q.prepare(R"(
      SELECT r.room_id,
             CASE WHEN r.user_a = ? THEN r.user_b ELSE r.user_a END AS peer_id,
             u.name AS peer_name,
             ( SELECT body FROM messages m WHERE m.room_id = r.room_id
               ORDER BY m.sent_at DESC LIMIT 1 ) AS last_body,
             ( SELECT sent_at FROM messages m WHERE m.room_id = r.room_id
               ORDER BY m.sent_at DESC LIMIT 1 ) AS last_time
      FROM dm_rooms r
      JOIN users u
        ON u.login_id =
           CASE WHEN r.user_a = ? THEN r.user_b ELSE r.user_a END
      WHERE r.user_a = ? OR r.user_b = ?
      ORDER BY last_time DESC, r.created_at DESC
    )");
    q.addBindValue(me);
    q.addBindValue(me);
    q.addBindValue(me);
    q.addBindValue(me);

    if (!q.exec()) { if (err) *err = q.lastError().text(); return out; }
    while (q.next()) {
        QVariantMap m;
        m["room_id"]   = q.value("room_id");
        m["peer_id"]   = q.value("peer_id");
        m["peer_name"] = q.value("peer_name");
        m["last_body"] = q.value("last_body");
        m["last_time"] = q.value("last_time");
        out.push_back(m);
    }
    return out;
}

bool DB::insertMessage(qint64 roomId, const QString& sender, const QString& body, QString* err)
{
    if (!ensureConnection(err)) return false;
    QSqlDatabase h = handle();
    QSqlQuery q(h);
    q.prepare("INSERT INTO messages(room_id, sender_login_id, body) VALUES (?,?,?)");
    q.addBindValue(roomId);
    q.addBindValue(sender);
    q.addBindValue(body);
    if (!q.exec()) { if (err) *err = q.lastError().text(); return false; }
    return true;
}

QList<QVariantMap> DB::listMessages(qint64 roomId, int limit, int offset, QString* err)
{
    QList<QVariantMap> out;
    if (!ensureConnection(err)) return out;
    QSqlDatabase h = handle();
    QSqlQuery q(h);
    q.prepare(R"(
        SELECT msg_id, sender_login_id, body, sent_at
        FROM messages
        WHERE room_id = ?
        ORDER BY sent_at ASC
        LIMIT ? OFFSET ?
    )");
    q.addBindValue(roomId);
    q.addBindValue(limit);
    q.addBindValue(offset);
    if (!q.exec()) { if (err) *err = q.lastError().text(); return out; }
    while (q.next()) {
        QVariantMap m;
        m["msg_id"] = q.value("msg_id");
        m["sender"] = q.value("sender_login_id");
        m["body"]   = q.value("body");
        m["sent_at"]= q.value("sent_at");
        out.push_back(m);
    }
    return out;
}

QString DB::getUserName(const QString& loginId, QString* err) {
    if (!ensureConnection(err)) return {};
    QSqlDatabase h = handle();
    QSqlQuery q(h);
    q.prepare("SELECT name FROM users WHERE login_id=? LIMIT 1");
    q.addBindValue(loginId);
    if (!q.exec()) { if (err) *err=q.lastError().text(); return {}; }
    return q.next() ? q.value(0).toString() : QString();
}

static const char* kConn = "talker_mysql";     // 고정 연결명

bool DB::ensureConnection(QString* err)
{
    // 이미 있으면 재사용
    if (QSqlDatabase::contains(kConn)) {
        QSqlDatabase db = QSqlDatabase::database(kConn);
        if (db.isOpen()) return true;
    }

    // 드라이버 체크
    if (!QSqlDatabase::isDriverAvailable("QMYSQL")) {
        if (err) *err = "QMYSQL driver not loaded";
        return false;
    }

    // 환경변수 → 없으면 기본값
    auto env = QProcessEnvironment::systemEnvironment();
    const QString host = env.value("DB_HOST", "127.0.0.1");
    const QString name = env.value("DB_NAME", "talker");
    const QString user = env.value("DB_USER", "root");
    const QString pass = env.value("DB_PASS", "");

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", kConn);
    db.setHostName(host);
    db.setDatabaseName(name);
    db.setUserName(user);
    db.setPassword(pass);
    db.setPort(3306);

    if (!db.open()) {
        if (err) *err = db.lastError().text();
        return false;
    }
    return true;
}

QSqlDatabase DB::handle()
{
    // 보장된 핸들 반환 (open 안되어 있으면 호출자가 ensureConnection() 먼저)
    return QSqlDatabase::database(kConn);
}

// 프로필 변경
bool DB::upsertStatusMessage(const QString& loginId,
                             const QString& status,
                             QString* errorOut)
{
    if (!connectTodb()) { if (errorOut) *errorOut = "DB 연결 실패"; return false; }

    QSqlQuery q(db);
    q.prepare(R"SQL(
        INSERT INTO user_profile(login_id, status_message)
        VALUES (:id, :msg)
        ON DUPLICATE KEY UPDATE
            status_message = VALUES(status_message),
            updated_at     = CURRENT_TIMESTAMP
    )SQL");
    q.bindValue(":id",  loginId.trimmed());
    q.bindValue(":msg", status.left(255).trimmed());

    if (!q.exec()) {
        if (errorOut) *errorOut = q.lastError().text();
        return false;
    }
    return true;
}

// 채팅방 삭제

bool DB::deleteDmRoom(qint64 roomId, const QString& me, QString* err)
{
    if (!connectTodb()) { if (err) *err = tr("DB 연결 실패"); return false; }
    QSqlDatabase h = db;

    if (!h.transaction()) {
        if (err) *err = tr("트랜잭션 시작 실패: %1").arg(h.lastError().text());
        return false;
    }

    {   // 내가 속한 방인지 검증
        QSqlQuery q(h);
        q.prepare(R"(SELECT 1 FROM dm_rooms WHERE room_id=? AND (user_a=? OR user_b=?) LIMIT 1)");
        q.addBindValue(roomId); q.addBindValue(me); q.addBindValue(me);
        if (!q.exec() || !q.next()) { h.rollback(); if (err) *err = tr("권한 없음/방 없음"); return false; }
    }
    {   // 메시지 삭제
        QSqlQuery q(h);
        q.prepare("DELETE FROM messages WHERE room_id=?");
        q.addBindValue(roomId);
        if (!q.exec()) { h.rollback(); if (err) *err = tr("메시지 삭제 실패: %1").arg(q.lastError().text()); return false; }
    }
    {   // 방 삭제
        QSqlQuery q(h);
        q.prepare("DELETE FROM dm_rooms WHERE room_id=?");
        q.addBindValue(roomId);
        if (!q.exec()) { h.rollback(); if (err) *err = tr("채팅방 삭제 실패: %1").arg(q.lastError().text()); return false; }
    }
    if (!h.commit()) { if (err) *err = tr("커밋 실패: %1").arg(h.lastError().text()); return false; }
    return true;
}


// 채팅방 목록 조회 쿼리: “내 상태” 반영

QList<QVariantMap> DB::fetchChatList(const QString& me, QString* err)
{
    QList<QVariantMap> rows;
    if (!connectTodb()) { if (err) *err = QObject::tr("DB 연결 실패"); return rows; }
    QSqlDatabase h = db;
    if (!ensureUserRoomStateTable(h, err)) return rows;

    QSqlQuery q(h);
    q.prepare(R"SQL(
      SELECT
        r.room_id,
        CASE WHEN r.user_a=? THEN r.user_b ELSE r.user_a END AS peer_id,
        COALESCE(u.name, '') AS peer_name,
        (
          SELECT m.body
          FROM messages m
          WHERE m.room_id = r.room_id
            AND (s.cleared_at IS NULL OR m.sent_at > s.cleared_at)
          ORDER BY m.sent_at DESC
          LIMIT 1
        ) AS last_body,
        (
          SELECT m.sent_at
          FROM messages m
          WHERE m.room_id = r.room_id
            AND (s.cleared_at IS NULL OR m.sent_at > s.cleared_at)
          ORDER BY m.sent_at DESC
          LIMIT 1
        ) AS last_time
      FROM dm_rooms r
      LEFT JOIN users u
        ON u.login_id = (CASE WHEN r.user_a=? THEN r.user_b ELSE r.user_a END)
      LEFT JOIN user_room_state s
        ON s.room_id = r.room_id AND s.login_id = ?
      WHERE (r.user_a=? OR r.user_b=?)
        AND COALESCE(s.hidden, 0) = 0            -- ✅ 숨긴 방 제외
      ORDER BY last_time IS NULL, last_time DESC
    )SQL");

    // 바인딩 순서(물음표 5개에 전부 me 바인딩)
    q.addBindValue(me);
    q.addBindValue(me);
    q.addBindValue(me);
    q.addBindValue(me);
    q.addBindValue(me);

    if (!q.exec()) { if (err) *err = q.lastError().text(); return rows; }

    while (q.next()) {
        QVariantMap r;
        r["room_id"]   = q.value(0).toLongLong();
        r["peer_id"]   = q.value(1).toString();
        r["peer_name"] = q.value(2).toString();
        r["last_body"] = q.value(3).toString();
        r["last_time"] = q.value(4).toDateTime();
        rows << r;
    }
    return rows;
}


// DB: 사용자별 상태 테이블 추가
bool DB::ensureUserRoomStateTable(QSqlDatabase& h, QString* err)
{
    QSqlQuery q(h);
    if (!q.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS user_room_state (
          room_id    BIGINT       NOT NULL,
          login_id   VARCHAR(64)  NOT NULL,
          hidden     TINYINT(1)   NOT NULL DEFAULT 0,
          cleared_at DATETIME     NULL,
          PRIMARY KEY (room_id, login_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )SQL")) {
        if (err) *err = q.lastError().text();
        return false;
    }
    return true;
}


// “방 나가기”를 전역 삭제 → 내 쪽만 숨김/초기화 로 변경
bool DB::leaveDmRoom(qint64 roomId, const QString& me, QString* err) {
    if (!connectTodb()) { if (err) *err = QObject::tr("DB 연결 실패"); return false; }
    QSqlDatabase h = db;
    if (!ensureUserRoomStateTable(h, err)) return false;

    // 내가 속한 방인지 안전 체크(옵션)
    {
        QSqlQuery q(h);
        q.prepare(R"(SELECT 1 FROM dm_rooms WHERE room_id=? AND (user_a=? OR user_b=?) LIMIT 1)");
        q.addBindValue(roomId); q.addBindValue(me); q.addBindValue(me);
        if (!q.exec() || !q.next()) { if (err) *err = QObject::tr("방 없음/권한 없음"); return false; }
    }

    // 내 쪽만 숨김 + cleared_at(초기화 시점 기록)
    QSqlQuery up(h);
    up.prepare(R"SQL(
        INSERT INTO user_room_state(room_id, login_id, hidden, cleared_at)
        VALUES(?, ?, 1, NOW())
        ON DUPLICATE KEY UPDATE hidden=VALUES(hidden), cleared_at=VALUES(cleared_at)
    )SQL");
    up.addBindValue(roomId);
    up.addBindValue(me);
    if (!up.exec()) { if (err) *err = up.lastError().text(); return false; }
    return true;
}

bool DB::unhideDmRoom(qint64 roomId, const QString& me, QString* err) {
    if (!connectTodb()) { if (err) *err = QObject::tr("DB 연결 실패"); return false; }
    QSqlDatabase h = db;
    if (!ensureUserRoomStateTable(h, err)) return false;

    QSqlQuery up(h);
    up.prepare(R"SQL(
        INSERT INTO user_room_state(room_id, login_id, hidden, cleared_at)
        VALUES(?, ?, 0, cleared_at)
        ON DUPLICATE KEY UPDATE hidden=0
    )SQL");
    up.addBindValue(roomId);
    up.addBindValue(me);
    if (!up.exec()) { if (err) *err = up.lastError().text(); return false; }
    return true;
}

bool DB::getClearedAt(qint64 roomId, const QString& me, QDateTime* out, QString* err) {
    if (out) *out = QDateTime();  // invalid
    if (!connectTodb()) { if (err) *err = QObject::tr("DB 연결 실패"); return false; }
    QSqlDatabase h = db;
    if (!ensureUserRoomStateTable(h, err)) return false;

    QSqlQuery q(h);
    q.prepare("SELECT cleared_at FROM user_room_state WHERE room_id=? AND login_id=?");
    q.addBindValue(roomId); q.addBindValue(me);
    if (!q.exec()) { if (err) *err = q.lastError().text(); return false; }
    if (q.next() && out) *out = q.value(0).toDateTime();
    return true;
}
