#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "db.h"                     // DB
#include "popup.h"                  // 팝업
#include "initialwidget.h"          // 초기
#include "loadingwidget.h"          // 로딩
#include "sign.h"                   // 회원가입
#include "find.h"                   // id/pw 찾기
#include "idfind.h"                 // id찾기
#include "pwfind.h"                 // pw찾기
#include "pwreset.h"                // pw 변경
#include "friendlist.h"             // 친구 목록
#include "setting.h"                // 설정
#include "more.h"                   // 더보기
#include "serch.h"                  // 검색
#include "serchok.h"                // 검색성공 - 친구추가
#include "frienddelete.h"           // 설정 - 친구삭제
#include "myprofile.h"              // 설정 - 프로필
#include "profilechange.h"          // 설정 - 프로필 변경
#include "chatlist.h"               // 채팅방 목록
#include "chatroom.h"               // 채팅방
#include "chatlistsetting.h"        // 채팅방 목록 - 설정
#include "chatlistitem.h"           // 채팅방 목록 - 설정 - 나가기
#include "chatlistleave.h"          // 채팅방 목록 - 설정 - 나가기 - 나가기 화면

#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLayout>
#include <QPushButton>
#include <QPixmap>
#include <QResizeEvent>
#include <QMessageBox>
#include <QGridLayout>
#include <QSpacerItem>
#include <QIcon>
#include <QResizeEvent>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QProcessEnvironment>

static const char* kLogoPath = ":/logo/logo.png";  // alias에 맞춤

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>

// C -> S
#define C2S_STATUS_NOTIFY "C2S_STATUS_NOTIFY"   // 내 상태메세지 바뀌었음을 서버에 알림

// S -> C
#define S2C_FRIEND_STATUS_CHANGED "S2C_FRIEND_STATUS_CHANGED" // 친구 상태메세지 변경 push

// 목업 스타일 1:1 채팅 확인 다이얼로그
static bool confirm1on1(QWidget* parent)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("채팅하기"));
    dlg.setModal(true);
    dlg.setFixedSize(360, 240);            // 목업 비율 비슷하게
    dlg.setSizeGripEnabled(false);

    // 전체 레이아웃
    auto root = new QVBoxLayout(&dlg);
    root->setContentsMargins(16, 14, 16, 16);
    root->setSpacing(10);

    // ── [본문] ─────────────────────────────────
    //  가운데 큰 이모지 + "1:1 채팅하기"
    auto center = new QVBoxLayout();
    center->setSpacing(8);

    auto bigIcon = new QLabel(&dlg);
    bigIcon->setText("💬");                 // 큰 채팅 이모지
    bigIcon->setAlignment(Qt::AlignCenter);
    bigIcon->setStyleSheet("font-size:50px;");

    auto title = new QLabel(QObject::tr("1:1 채팅하기"), &dlg);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:18px; font-weight:700;");

    center->addStretch(1);
    center->addWidget(bigIcon);
    center->addWidget(title);
    center->addStretch(1);
    root->addLayout(center, 1);

    // ── [버튼줄] ────────────────────────────────
    auto btns = new QHBoxLayout();
    auto yes = new QPushButton(QObject::tr("예"), &dlg);
    auto no  = new QPushButton(QObject::tr("아니오"), &dlg);

    yes->setMinimumSize(92, 36);
    no->setMinimumSize(92, 36);
    yes->setDefault(true);                 // Enter 기본값
    yes->setAutoDefault(true);

    // 버튼 스타일(목업 느낌)
    yes->setStyleSheet(
        "QPushButton{background:#ffffff; border:1px solid #e25740; border-radius:6px;}"
        "QPushButton:hover{background:#fff3f1;}"
        );
    no->setStyleSheet(
        "QPushButton{background:#ffffff; border:1px solid #cfcfcf; border-radius:6px;}"
        "QPushButton:hover{background:#f4f6f8;}"
        );

    btns->addStretch(1);
    btns->addWidget(yes);
    btns->addSpacing(12);
    btns->addWidget(no);
    btns->addStretch(1);
    root->addLayout(btns);

    // 다이얼로그 전반 색감
    dlg.setStyleSheet(
        "QDialog{background:#ffffff;}"
        "QLabel{color:#222;}"
        );

    QObject::connect(yes, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(no,  &QPushButton::clicked, &dlg, &QDialog::reject);

    return dlg.exec() == QDialog::Accepted;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)   // ← 생성자 타입 일치
{
    ui->setupUi(this);

    setWindowTitle(QStringLiteral("로그인 화면"));


    // 상단 중앙 로고(QLabel: objectName=logo)
    ui->logo->setAlignment(Qt::AlignCenter);
    ui->logo->setMinimumSize(200, 200);

    updateLogo(); // 초기 1회 적용

    // --- 페이지 생성 ---
    auto stacked = ui->stackedWidget;

    // 디자이너에 이미 있는 '로그인' 페이지의 인덱스 (보통 0)
    idxLogin_ = 0;

    // 1) 멤버로 쓰는 페이지들(여러 곳에서 써서 포인터 멤버로 유지)
    friendlist_ = new friendList(this);   // 친구 목록
    serch_      = new Serch(this);        // 검색
    serchok_    = new SerchOk(this);      // 친구추가(검색 성공)
    friendDelete_ = new FriendDelete(this);     //친구삭제
    myProfile_ = new MyProfile(this);           // 프로필
    profileChange_ = new ProfileChange(this);   // 프로필 변경
    chatList_ = new ChatList(this);             // 채팅방 목록
    chatRoom_ = new ChatRoom(this);             // 채팅방
    chatListSetting_ = new ChatListSetting(this); // 채팅방 목록 - 설정
    chatListLeave_   = new ChatListLeave(this);

    // 2) 지역으로만 써도 되는 페이지들
    auto initial = new InitialWidget(this);   // 초기
    auto loading = new LoadingWidget(this);   // 로딩
    auto signup  = new Sign(this);            // 회원가입
    auto find    = new Find(this);            // ID/PW 찾기
    auto idfind  = new idFind(this);          // ID 찾기
    auto pwfind  = new pwFind(this);          // PW 찾기
    auto pwreset = new pwReset(this);         // PW 변경
    auto setting = new Setting(this);         // 설정
    auto more    = new More(this);            // 더보기


    // --- 스택에 "정확히 한 번씩" 추가하고 인덱스 저장 ---
    idxInitial_    = stacked->addWidget(initial);
    idxLoading_    = stacked->addWidget(loading);
    idxSignup_     = stacked->addWidget(signup);
    idxFind_       = stacked->addWidget(find);
    idxIdFind_     = stacked->addWidget(idfind);
    idxPwFind_     = stacked->addWidget(pwfind);
    idxPwReset_    = stacked->addWidget(pwreset);
    idxFriendList_ = stacked->addWidget(friendlist_);
    idxSetting_    = stacked->addWidget(setting);
    idxMore_       = stacked->addWidget(more);
    idxSerch_      = stacked->addWidget(serch_);
    idxSerchOk_    = stacked->addWidget(serchok_);
    idxFriendDelete_= stacked->addWidget(friendDelete_);
    idxMyProfile_ = ui->stackedWidget->addWidget(myProfile_);
    idxChatList_ = ui->stackedWidget->addWidget(chatList_);
    idxChatRoom_ = ui->stackedWidget->addWidget(chatRoom_);
    idxProfileChange_ = ui->stackedWidget->addWidget(profileChange_);
    idxChatListSetting_ = ui->stackedWidget->addWidget(chatListSetting_);
    idxChatListLeave_ = ui->stackedWidget->addWidget(chatListLeave_);


    // 화면 추가

    // 1.시작은 초기 화면
    ui->stackedWidget->setCurrentIndex(idxInitial_);

    // 2. 초기 로고 클릭 → 로딩 시작
    connect(initial, &InitialWidget::startRequested, this, [this, loading]{
        ui->stackedWidget->setCurrentIndex(idxLoading_);
        loading->start();
    });

    // 3.로딩 완료 → 로그인
    connect(loading, &LoadingWidget::finished, this, &MainWindow::goToLogin);

    // 4. 로그인 화면 → 로그인 버튼 클릭
    connect(ui->loginBtn, &QPushButton::clicked, this, &MainWindow::handleLogin);

    // 로그인 화면에서 Enter로 로그인: ✅ 생성자에서 1회만 연결
    connect(ui->pwEdit, &QLineEdit::returnPressed, this, &MainWindow::handleLogin);
    connect(ui->idEdit, &QLineEdit::returnPressed, this, &MainWindow::handleLogin);

    // 입력창들 비밀번호 모드 보장
    ui->pwEdit->setEchoMode(QLineEdit::Password);

    // 5.로그인 화면 → 회원가입 버튼 → 회원가입 화면
    connect(ui->signupBtn, &QPushButton::clicked, this, [this]{
        ui->stackedWidget->setCurrentIndex(idxSignup_);
    });

    // 6.로그인 화면 → ID/PW 찾기 버튼 → ID/PW찾기 화면
    connect(ui->findBtn,   &QPushButton::clicked, this, [this]
            { ui->stackedWidget->setCurrentIndex(idxFind_); });

    // 7.로그인 화면 → 종료 버튼 → 종료
    connect(ui->exitBtn,   &QPushButton::clicked, this, &MainWindow::exitApp);

    // 8.회원가입 → 취소 버튼 → 로그인 화면
    connect(signup, &Sign::back, this, &MainWindow::goToLogin);

    // 9.회원가입 → 가입하기 버튼 → 로그인 화면
    connect(signup, &Sign::join, this, &MainWindow::goToLogin);

    // 9.ID/PW 찾기 화면 → 취소 버튼 → 로그인 화면
    connect(find, &Find::back, this, &MainWindow::goToLogin);

    // 10.ID/PW 찾기 화면 → ID찾기 버튼 → ID찾기 화면
    connect(find, &Find::idFind, this, [this, stacked, idfind]{
        stacked->setCurrentWidget(idfind);
    });

    // 11.ID 찾기 화면 → 취소 버튼 → ID/PW 찾기 화면
    connect(idfind, &idFind::back, this, [this, stacked, find]{
        stacked->setCurrentWidget(find);
    });

    // 12.ID 찾기 화면 → 찾기 버튼 → 유효성 검사 완료 후 로그인 화면
    connect(idfind, &idFind::findBtn, this, &MainWindow::goToLogin);

    // 13.ID/PW 찾기 화면 → PW찾기 버튼 → PW찾기 화면
    connect(find, &Find::pwFind, this, [this, stacked, pwfind]{
        stacked->setCurrentWidget(pwfind);
    });

    // 14.PW 찾기 화면 → 취소 버튼 → ID/PW 찾기 화면
    connect(pwfind, &pwFind::back, this, [this, stacked, find]{
        stacked->setCurrentWidget(find);
    });

    // 15.PW 찾기 → (검증 성공) → 찾기 버튼 → PW변경 화면으로 전환
    connect(pwfind, &pwFind::pwResetRequested, this,
            [this, pwreset](const QString& id){
                pwreset->setLoginId(id);
                ui->stackedWidget->setCurrentWidget(pwreset);   // ✅ 인덱스 대신 포인터
            });

    // PW변경 완료 → 로그인 화면 복귀
    connect(pwreset, &pwReset::done, this, [this]{
        // 로그인 입력칸 정리
        ui->idEdit->clear();
        ui->pwEdit->clear();

        // 로그인 화면으로 전환
        goToLogin();

        // 포커스는 ID 입력칸으로
        ui->idEdit->setFocus();
    });

    // 채팅 탭 눌렀을 때 항상 최신 목록 로드
    connect(chatList_, &ChatList::chatTabRequested, this, [this]{
        chatList_->reload(currentLoginId_);                // ✅
        ui->stackedWidget->setCurrentWidget(chatList_);
    });

    // 친구목록에서 “채팅” 버튼 눌러 들어갈 때도 로드
    connect(friendlist_, &friendList::chat, this, [this]{
        chatList_->reload(currentLoginId_);
        ui->stackedWidget->setCurrentWidget(chatList_);
    });

    // 친구 목록 → 검색 버튼 → 검색화면
    connect(friendlist_, &friendList::serch, this, [this]{
        serch_->setMyId(currentLoginId_);                 // 자기 자신 검색 방지용
        ui->stackedWidget->setCurrentWidget(serch_);
    });

    // 검색 화면 → 취소 버튼 → 친구 목록 화면
    connect(serch_, &Serch::back, this, [this]{
        ui->stackedWidget->setCurrentWidget(friendlist_);
    });

    // 검색 성공 → 친구 추가 화면
    connect(serch_, &Serch::candidateReady, this,
            [this](const QString& friendId, const QString& name, const QString& status){
                serchok_->setProfile(friendId, name, status);
                ui->stackedWidget->setCurrentWidget(serchok_);
            });

    // 친구추가 버튼 처리
    connect(serchok_, &SerchOk::addFriendRequested, this,
            [this](const QString& friendId){
                if (currentLoginId_.isEmpty()) {
                    Popup::error(this, tr("로그인 세션이 만료되었습니다."), tr("오류"));
                    return;
                }
                DB db(this);
                QString err;
                if (!db.addFriend(currentLoginId_, friendId, &err)) {
                    Popup::error(this, tr("친구 추가 실패: %1").arg(err), tr("오류"));
                    return;
                }
                if (friendlist_) friendlist_->refreshFromDb(currentLoginId_);  // ✅ 멤버
                Popup::success(this, tr("친구를 추가했습니다."), tr("완료"));
                ui->stackedWidget->setCurrentWidget(friendlist_);
            });


    // 친구 추가 화면 → 취소 버튼 → 검색화면
    connect(serchok_, &SerchOk::back, this, [this]{
        ui->stackedWidget->setCurrentWidget(serch_);
    });

    // 채팅방 목록 하단 탭 → 각 화면으로
    connect(chatList_, &ChatList::friendTabRequested, this, [this]{
        ui->stackedWidget->setCurrentWidget(friendlist_);
    });

    connect(chatList_, &ChatList::chatTabRequested, this, [this]{
        chatList_->reload(currentLoginId_);              // ✅
        ui->stackedWidget->setCurrentWidget(chatList_);
    });

    connect(chatList_, &ChatList::openRoomRequested, this,
            [this](qint64 roomId, const QString& peerId, const QString& peerName){
                chatRoom_->open(roomId, currentLoginId_, peerId, peerName);
                ui->stackedWidget->setCurrentWidget(chatRoom_);
            });

    connect(chatList_, &ChatList::moreTabRequested, this, [this, more]{
        ui->stackedWidget->setCurrentWidget(more);
    });

    // 친구 목록 → 설정 버튼 → 설정화면
    connect(friendlist_, &friendList::setting, this, [this, stacked, setting]{
        stacked->setCurrentWidget(setting);
    });

    // 친구 목록 → 채팅 버튼 → 채팅방 목록
    connect(friendlist_, &friendList::chat, this, [this]{
        ui->stackedWidget->setCurrentIndex(idxChatList_);
    });

    // 설정화면 → 친구삭제 버튼 → 친구삭제 화면
    connect(setting, &Setting::frienddelet, this, [this]{
        if (currentLoginId_.isEmpty()) {
            Popup::error(this, tr("로그인 세션이 만료되었습니다."), tr("오류"));
            return;
        }
        friendDelete_->setOwnerId(currentLoginId_);          // 목록 로드
        ui->stackedWidget->setCurrentWidget(friendDelete_);  // 화면 전환
    });

    // 친구삭제 화면 → 취소버튼 → 설정화면
    connect(friendDelete_, &FriendDelete::back, this, [this]{
        ui->stackedWidget->setCurrentIndex(idxSetting_);
    });

    // 친구삭제 화면 → 한 명 삭제될 때 → 친구목록 최신화만
    connect(friendDelete_, &FriendDelete::deletedOne, this, [this]{
        if (friendlist_ && !currentLoginId_.isEmpty())
            friendlist_->refreshFromDb(currentLoginId_);
        ui->stackedWidget->setCurrentWidget(friendlist_);   // ← 친구목록 화면으로 이동
    });

    // 설정 → 새로고침
    connect(setting, &Setting::refresh, this, [this]{
        if (friendlist_ && !currentLoginId_.isEmpty())
            friendlist_->refreshFromDb(currentLoginId_);
        Popup::success(this, tr("최신 상태메세지로 갱신했습니다."), tr("완료"));
        // 갱신 후 바로 친구목록으로
        ui->stackedWidget->setCurrentWidget(friendlist_);
    });

    // 설정 → 프로필 버튼 → 프로필 화면
    connect(setting, &Setting::profile, this, [this]{
        if (currentLoginId_.isEmpty()) {
            Popup::error(this, tr("로그인이 필요합니다."), tr("오류"));
            return;
        }
        myProfile_->setLoginId(currentLoginId_);
        ui->stackedWidget->setCurrentIndex(idxMyProfile_);
    });

    // 프로필화면 → 나가기 버튼 → 설정화면으로 복귀
    connect(myProfile_, &MyProfile::back, this, [this]{
        ui->stackedWidget->setCurrentIndex(idxSetting_);
    });

    // ── 설정 화면의 "프로필 변경" 버튼 → 새 화면으로 이동
    connect(setting, &Setting::profilechange, this, [this]{
        if (currentLoginId_.isEmpty()) {
            Popup::error(this, tr("로그인이 필요합니다."), tr("오류"));
            return;
        }

        // 현재 내 이름/상태 읽어와서 초기값 표시
        QString curName, curStatus;
        {
            DB db(this);
            db.getUserProfileById(currentLoginId_, &curName, &curStatus); // 프로젝트에 있는 함수
        }
        profileChange_->setInitial(curName, curStatus);
        ui->stackedWidget->setCurrentWidget(profileChange_);
    });

    // 프로필변경 → 취소 → 설정 화면으로 복귀
    connect(profileChange_, &ProfileChange::cancelRequested, this, [this]{
        ui->stackedWidget->setCurrentIndex(idxSetting_);
    });


    // 프로필변경 → 저장 → 화면 갱신 → 설정 화면으로 복귀
    connect(profileChange_, &ProfileChange::applyRequested, this, [this](const QString& text){
        if (text.isEmpty()) {
            Popup::error(this, tr("상태메세지를 입력하세요."), tr("입력 오류"));
            return;
        }

        DB db(this);
        QString err;
        if (!db.upsertStatusMessage(currentLoginId_, text, &err)) {
            Popup::error(this, tr("저장 실패: %1").arg(err), tr("오류"));
            return;
        }

        // 화면 갱신
        if (friendlist_) friendlist_->refreshFromDb(currentLoginId_);
        if (myProfile_)  myProfile_->setLoginId(currentLoginId_);

        Popup::success(this, tr("상태메세지를 변경했습니다."), tr("성공"));

        if (clientSocket_ && clientSocket_->state() == QAbstractSocket::ConnectedState) {
            QJsonObject obj{{"type","C2S_STATUS_NOTIFY"}, {"status", text}};
            clientSocket_->write(QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n"); // ← \n 필수
        }

        // 2) 설정 화면으로 복귀
        ui->stackedWidget->setCurrentIndex(idxSetting_);
    });

    // 설정화면 → 취소 버튼 → 친구목록
    connect(setting, &Setting::back, this, [this]{
        ui->stackedWidget->setCurrentWidget(friendlist_);
    });

    // 친구 목록 → 더보기 버튼 → 더보기화면
    connect(friendlist_, &friendList::more, this, [this, stacked, more]{
        stacked->setCurrentWidget(more);
    });

    // 더보기화면 → 취소 버튼 → 친구목록
    connect(more, &More::back, this, [this]{
        ui->stackedWidget->setCurrentWidget(friendlist_);
    });

    // 더보기화면 → 로그아웃 버튼 → 로그인화면
    connect(more, &More::logout, this, &MainWindow::goToLogin);

    // 친구 카드 클릭 → 팝업 → 방 보장 → 방 입장
    connect(friendlist_, &friendList::friendClicked, this,
            [this](const QString& fid, const QString& fname){
                if (!confirm1on1(this)) {                     // 아니오 → 친구목록 유지
                    ui->stackedWidget->setCurrentWidget(friendlist_);
                    return;
                }
                // 예 → 방 보장(DB 저장) → 목록/화면 생성 → 입장
                DB db(this);
                qint64 roomId = 0; QString err;
                const int mySock = clientSocket_ ? clientSocket_->socketDescriptor() : 0;
                const QString myIp = clientSocket_ ? clientSocket_->localAddress().toString() : QString();

                if (!db.ensureDmRoom(currentLoginId_, fid, mySock, myIp, &roomId, &err)) {
                    Popup::error(this, tr("채팅방 생성 실패: %1").arg(err), tr("오류"));
                    return;
                }
                // 채팅방 목록에도 즉시 나타나도록
                chatList_->reload(currentLoginId_);
                // 채팅방 입장
                chatRoom_->open(roomId, currentLoginId_, fid, fname);
                ui->stackedWidget->setCurrentWidget(chatRoom_);
            });

    // 채팅방
    connect(chatRoom_, &ChatRoom::backRequested, this, [this]{
        chatList_->reload(currentLoginId_);              // 최신 메시지 반영
        ui->stackedWidget->setCurrentWidget(chatList_);  // 채팅방 목록으로
    });

    // 채팅목록의 톱니 → 설정 화면
    connect(chatList_, &ChatList::openSettingRequested, this, [this]{
        ui->stackedWidget->setCurrentWidget(chatListSetting_);
    });

    // 설정의 취소 → 채팅목록 복귀
    connect(chatListSetting_, &ChatListSetting::cancelRequested, this, [this]{
        chatList_->reload(currentLoginId_);               // ← DB에서 다시 읽기(hidden 반영)
        ui->stackedWidget->setCurrentWidget(chatList_);   // 목록 화면으로 전환
    });

    // 설정의 "채팅방 나가기" → 나가기 화면 생성/로딩 후 전환
    connect(chatListSetting_, &ChatListSetting::leaveModeRequested, this, [this]{
        chatListLeave_->reload(currentLoginId_);     // ← 평소 ChatList.reload에 쓰던 로그인ID 변수 사용
        ui->stackedWidget->setCurrentWidget(chatListLeave_);
    });

    // 나가기 화면(ChatListLeave)에서 방 나감 성공 시 → 설정화면으로 복귀하기 전에 미리 로드
    connect(chatListLeave_, &ChatListLeave::leftOne, this, [this]{
        chatList_->reload(currentLoginId_);               // ← 미리 최신화
        ui->stackedWidget->setCurrentWidget(chatListSetting_);
    });

    // 설정 화면의 취소 → 채팅 목록으로 복귀할 때 반드시 새로고침:

    connect(chatListSetting_, &ChatListSetting::cancelRequested, this, [this]{
        chatList_->reload(currentLoginId_);                // ✅ 목록 재조회 (hidden 필터 반영)
        ui->stackedWidget->setCurrentWidget(chatList_);
    });

    // 나가기 화면 하단 취소 → 다시 "설정 화면"으로
    connect(chatListLeave_, &ChatListLeave::cancelRequested, this, [this]{
        ui->stackedWidget->setCurrentWidget(chatListSetting_);
    });

    // 채팅방에서 보냄/수신 발생 시 → 채팅방 목록 즉시 갱신
    connect(chatRoom_, &ChatRoom::messageActivity,
            chatList_,  &ChatList::onMessageActivity);

    // (‘채팅방 나가기’ 별도 화면을 만들었다면)
    connect(chatRoom_, &ChatRoom::messageActivity,
            chatListLeave_, &ChatListLeave::onMessageActivity);
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::goToSignup(){ ui->stackedWidget->setCurrentIndex(idxSignup_); }
void MainWindow::goToFind()  { ui->stackedWidget->setCurrentIndex(idxFind_);  }
void MainWindow::goToLogin() { ui->stackedWidget->setCurrentIndex(idxLogin_); }
void MainWindow::exitApp(){ close(); }


// 로그인
void MainWindow::handleLogin()
{
    // 1) 입력값 가져오기
    const QString loginId = ui->idEdit->text().trimmed();  // UI의 ID 입력필드
    const QString password = ui->pwEdit->text().trimmed(); // UI의 PW 입력필드

    // 2) 기본 유효성 검사
    if (loginId.isEmpty()) {
        Popup::error(this, tr("ID를 입력해주세요."), tr("로그인 실패"));
        ui->idEdit->setFocus();
        return;
    }

    if (password.isEmpty()) {
        Popup::error(this, tr("비밀번호를 입력해주세요."), tr("로그인 실패"));
        ui->pwEdit->setFocus();
        return;
    }

    // ID 형식 검증 (영문, 숫자만 허용)
    static const QRegularExpression idSpecialCharRx(R"([^A-Za-z0-9])");
    if (idSpecialCharRx.match(loginId).hasMatch()) {
        Popup::error(this, tr("ID는 영문, 숫자만 입력해주세요."), tr("로그인 실패"));
        ui->idEdit->setFocus();
        return;
    }

    // ID 길이 검증 (8자리 이상)
    if (loginId.length() < 8) {
        Popup::error(this, tr("ID는 8자리 이상이어야 합니다."), tr("로그인 실패"));
        ui->idEdit->setFocus();
        return;
    }

    // 3) DB 검증
    DB db(this);
    if (db.verifyLogin(loginId, password)) {
        // 로그인 성공
        Popup::success(this,
                       tr("%1님, 환영합니다!").arg(loginId),
                       tr("로그인 성공"));

        currentLoginId_ = loginId;
        if (friendlist_) friendlist_->refreshFromDb(currentLoginId_);  // ✅ 지역변수 X, 멤버 O
        ui->stackedWidget->setCurrentIndex(idxFriendList_);

        // 입력 필드 초기화
        ui->idEdit->clear();
        ui->pwEdit->clear();

        // 소켓 생성 + readyRead 연결은 로그인 성공 시 한 번만
        if (!clientSocket_) {
            clientSocket_ = new QTcpSocket(this);
            connect(clientSocket_, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
        }

        if (clientSocket_->state() != QAbstractSocket::ConnectedState) {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            const QString host = env.value("CHAT_HOST", "127.0.0.1");
            const quint16 port = env.value("CHAT_PORT", "5555").toUShort();

            clientSocket_->connectToHost(host, port);
            if (!clientSocket_->waitForConnected(2000)) {
                Popup::error(this, tr("채팅 서버 연결 실패: %1").arg(clientSocket_->errorString()), tr("연결 오류"));
                return;
            }

            QJsonObject obj{{"type","login"},{"login_id", currentLoginId_}};
            clientSocket_->write(QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n");
        }
        // 채팅 모듈에 소켓 주입
        chatRoom_->setSocket(clientSocket_);
        // chatList_->reload(currentLoginId_);

    } else {
        // 로그인 실패
        Popup::error(this,
                     tr("ID 또는 비밀번호가 올바르지 않습니다."),
                     tr("로그인 실패"));

        // 비밀번호 필드만 초기화 (보안상 이유)
        ui->idEdit->clear();
        ui->pwEdit->clear();
    }
}



void MainWindow::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateLogo(); // 라벨 크기 변경 시 다시 맞춤
}

void MainWindow::updateLogo()
{
    if (!QFile::exists(kLogoPath)) {
        qWarning() << "[MainWindow] 로고 로드 실패:" << kLogoPath;
        return;
    }
    QPixmap raw(kLogoPath);
    if (raw.isNull()) {
        qWarning() << "[MainWindow] 로고 로드 실패(Null):" << kLogoPath;
        return;
    }
    auto sz = ui->logo->size();
    if (sz.isEmpty()) sz = QSize(240,240);
    QPixmap scaled = raw.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->logo->setPixmap(scaled);
    ui->logo->setAlignment(Qt::AlignCenter);
}

void MainWindow::onSocketReadyRead() {
    while (clientSocket_ && clientSocket_->canReadLine()) {
        const QByteArray line = clientSocket_->readLine().trimmed();
        if (line.isEmpty()) continue;

        QJsonParseError pe{};
        const QJsonDocument doc = QJsonDocument::fromJson(line, &pe);
        if (pe.error != QJsonParseError::NoError || !doc.isObject()) continue;

        const QJsonObject obj = doc.object();
        const QString type = obj.value("type").toString();

        if (type == S2C_FRIEND_STATUS_CHANGED) {
            qDebug() << "push status changed from" << obj.value("login_id").toString();
            const QString who    = obj.value("login_id").toString();
            const QString status = obj.value("status").toString();

            // 가장 간단한 방법: 통째로 새로고침 (이미 있는 함수 재사용)
            if (friendlist_ && !currentLoginId_.isEmpty()) {
                friendlist_->refreshFromDb(currentLoginId_);
            }

            // 더 정교하게 하려면 friendlist_에 updateFriendStatus(who, status) 같은
            // 메서드를 만들어 해당 카드의 라벨만 바꿔도 된다.
        }

        else if (type == "chat") {
            const qint64 rid = static_cast<qint64>(obj.value("room_id").toDouble());
            const QString txt = obj.value("text").toString();
            QDateTime when = QDateTime::fromString(obj.value("ts").toString(), Qt::ISODate);
            if (!when.isValid()) when = QDateTime::currentDateTime();

            // 👇 내 클라이언트 기준: "내가 수신자"이고 예전에 내가 나간 상태(hidden=1)였다면,
            // 새 메시지를 받으면서 자동으로 숨김 해제
            {
                DB db(this);
                db.unhideDmRoom(rid, currentLoginId_, nullptr);
            }

            // 지금 화면이 채팅방이고, 그 채팅방의 id가 rid와 같다면 → 채팅방에 전달
            const bool chatRoomVisible = (ui->stackedWidget->currentWidget() == chatRoom_);
            const bool sameRoom = (chatRoom_ && chatRoom_->currentRoomId() == rid);

            if (chatRoomVisible && sameRoom) {
                // ChatRoom이 appendBubble을 하고, 그 직후에 messageActivity를 emit 하도록 되어 있음
                chatRoom_->handleServerMessage(obj);
            } else {
                // 방이 열려 있지 않으면 목록을 직접 갱신 (미리보기/시간 업데이트 + 최상단 이동)
                if (chatList_) chatList_->onMessageActivity(rid, txt, when);
                if (chatListLeave_) chatListLeave_->onMessageActivity(rid, txt, when);
            }
        }
    }
}
