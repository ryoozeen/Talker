#include <QCoreApplication>     // 콘솔 전용 Qt 애플리케이션 (GUI 없음)
#include <QCommandLineParser>   // 명령줄 인자 파서 (호스트/포트 옵션 처리)
#include <QHostAddress>         // IP/포트 다룰 때 사용
#include "chatserver.h"
#include "dbutil.h"

int main(int argc, char *argv[])
{
    // QCoreApplication: 이벤트 루프가 있는 콘솔 앱 베이스
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("TalkerChatServer");
    QCoreApplication::setOrganizationName("Talker");

    // ─────────────────────────────────────────────
    // 명령줄 옵션: --host / --port
    // ─────────────────────────────────────────────
    QCommandLineParser parser;
    parser.addHelpOption();  // --help 지원

    // 예: --host 0.0.0.0  (기본값 0.0.0.0: 모든 인터페이스 바인딩)
    QCommandLineOption hostOpt({"H","host"}, "Listen host (default 0.0.0.0)", "host", "0.0.0.0");

    // 예: --port 5555     (기본값 5555)
    QCommandLineOption portOpt({"p","port"}, "Listen port (default 5555)", "port", "5555");
    parser.addOption(hostOpt);
    parser.addOption(portOpt);
    parser.process(app);

    // ─────────────────────────────────────────────
    // DB 연결 (환경변수 또는 기본값으로 접속)
    // - DB_HOST, DB_NAME, DB_USER, DB_PASS
    // - 기본값: 127.0.0.1 / talker / root / 빈 비번
    // ─────────────────────────────────────────────

    // DB 연결 실패 알림
    if (!DbUtil::connectFromEnv()) {
        qCritical() << "DB connect failed. Set env: DB_HOST, DB_NAME, DB_USER, DB_PASS";
        return 1;  // 실패 시 종료
    }

    // ─────────────────────────────────────────────
    // TCP 서버 시작
    // ─────────────────────────────────────────────
    ChatServer server;
    const auto host = QHostAddress(parser.value(hostOpt));   // 문자열 → QHostAddress
    const quint16 port = parser.value(portOpt).toUShort();   // 문자열 → 포트 숫자
    if (!server.listen(host, port)) {
        qCritical() << "Listen failed:" << server.errorString();
        return 1;
    }
    qInfo() << "ChatServer listening on" << host.toString() << port;

    // 이벤트 루프 진입
    return app.exec();
}
