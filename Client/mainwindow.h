#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QString>      // ✅ QString 값 멤버에 필요
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>

// ✅ 페이지 클래스 전방선언(헤더 의존성 최소화)
class Serch;
class SerchOk;
class friendList;
class FriendDelete;
class MyProfile;
class ChatList;
class ChatRoom;
class QResizeEvent;     // ✅ 포인터 사용이므로 전방선언으로 충분
class ProfileChange;
class ChatListSetting;
class ChatListItem;
class ChatListLeave;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow  // ← 클래스명도 MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void goToSignup();
    void goToFind();
    void goToLogin();
    void exitApp();
    void handleLogin();    // ✅ 로그인 처리 함수 추가
    void onSocketReadyRead();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;

    // ✅ 스택 인덱스 멤버 (하드코딩 금지)
    int idxLogin_   = -1;  // 로그인 페이지(디자이너 쪽 기본 페이지)
    int idxInitial_ = -1;
    int idxLoading_ = -1;
    int idxSignup_  = -1;
    int idxFind_    = -1;
    int idxIdFind_  = -1;
    int idxPwFind_  = -1;
    int idxPwReset_ = -1;
    int idxFriendList_ = -1;
    int idxSetting_ = -1;
    int idxMore_    = -1;
    int idxSerch_   = -1;
    int idxSerchOk_ = -1;
    int idxFriendDelete_ = -1;
    int idxMyProfile_ = -1;
    int idxChatList_ = -1;
    int idxChatRoom_ = -1;
    int idxProfileChange_ = -1;
    int idxChatListSetting_ = -1;
    int idxChatListItem_ = -1;
    int idxChatListLeave_ = -1;

    void updateLogo();

    // ✅ 로그인 정보/페이지 포인터
    QString    currentLoginId_;   // 로그인 성공 시 저장

    Serch*     serch_   = nullptr;
    SerchOk*   serchok_ = nullptr;
    friendList* friendlist_  = nullptr;
    FriendDelete* friendDelete_  = nullptr;
    MyProfile* myProfile_  = nullptr;
    ChatList* chatList_ = nullptr;
    ChatRoom* chatRoom_ = nullptr;
    ProfileChange* profileChange_ = nullptr;   // 새 화면
    ChatListSetting* chatListSetting_ = nullptr;
    ChatListItem* chatListItem_ = nullptr;
    ChatListLeave* chatListLeave_ = nullptr;


    QTcpSocket* clientSocket_ = nullptr; // 서버와 연결 소켓(없으면 추가)

};

#endif // MAINWINDOW_H
