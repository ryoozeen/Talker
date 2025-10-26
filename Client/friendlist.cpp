#include "friendlist.h"
#include "ui_friendlist.h"
#include "db.h"
#include "popup.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSizePolicy>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>

// ⬇️ 고정 카드 높이(원하는 값으로 조절 가능: 72~84 추천)
static constexpr int kCardHeight = 72;

friendList::friendList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::friendList)
{
    ui->setupUi(this);

    // 세팅
    connect(ui->settingBtn, &QPushButton::clicked,
            this,           &friendList::setting);

    // 검색
    connect(ui->serchBtn, &QPushButton::clicked,
            this,           &friendList::serch);

    // 채팅
    connect(ui->chatBtn, &QPushButton::clicked,
            this,           &friendList::chat);

    // 더보기
    connect(ui->moreBtn, &QPushButton::clicked,
            this,           &friendList::more);

    // 친구
    connect(ui->friendBtn, &QPushButton::clicked,
            this,           &friendList::friendlist);


}

friendList::~friendList()
{
    delete ui;
}

void friendList::appendFriendCard(const QString& friendId, const QString& name, const QString& status) {
    auto area = ui->friendlist;
    QWidget* content = area->widget();
    if (!content) {
        content = new QWidget(area);
        area->setWidget(content);
        area->setWidgetResizable(true);
    }
    auto layout = qobject_cast<QVBoxLayout*>(content->layout());
    if (!layout) {
        layout = new QVBoxLayout(content);
        layout->setContentsMargins(8,8,8,8);
        layout->setSpacing(6);
        content->setLayout(layout);
    }

    auto card = makeProfileCard(
        QString("%1 (%2)").arg(name, friendId),
        status,
        /*emphasize=*/false,
        content
        );
    layout->addWidget(card);

    // ⬇️ 오버레이 버튼 제거하고, 카드 자체를 클릭 타깃으로 사용
    card->setProperty("fid",   friendId);
    card->setProperty("fname", name);
    card->setCursor(Qt::PointingHandCursor);
    card->installEventFilter(this);   // ⬅️ 카드 클릭을 이 위젯에서 처리
}

bool friendList::eventFilter(QObject* obj, QEvent* ev)
{
    // 프로필 카드만 클릭 처리
    if (auto w = qobject_cast<QWidget*>(obj)) {
        if (w->objectName() == QStringLiteral("profileCard")) {
            if (ev->type() == QEvent::MouseButtonRelease) {
                auto me = static_cast<QMouseEvent*>(ev);
                if (me->button() == Qt::LeftButton) {
                    const QString fid   = w->property("fid").toString();
                    const QString fname = w->property("fname").toString();
                    if (!fid.isEmpty()) {
                        emit friendClicked(fid, fname);   // ⬅️ 시그널 발사
                        return true; // 이벤트 소비
                    }
                }
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void friendList::refreshFromDb(const QString& myId) {
    ownerId_ = myId;

    auto area = ui->friendlist;
    QWidget* content = area->widget();
    if (!content) {
        content = new QWidget(area);
        area->setWidget(content);
        area->setWidgetResizable(true);
    }
    auto layout = qobject_cast<QVBoxLayout*>(content->layout());
    if (!layout) {
        layout = new QVBoxLayout(content);
        layout->setContentsMargins(8,8,8,8);
        layout->setSpacing(6);
        content->setLayout(layout);
    }

    // 기존 위젯 제거
    while (QLayoutItem* it = layout->takeAt(0)) {
        if (auto w = it->widget()) w->deleteLater();
        delete it;
    }

    // 1) 내 프로필 카드 (맨 위에 고정 한 장)
    {
        DB db(this->window());
        QString myName, myStatus;
        if (db.getUserProfileById(myId, &myName, &myStatus)) {
            auto me = makeProfileCard(tr("나") + QString(" (%1)").arg(myId), myStatus, true, content);
            layout->addWidget(me);
        }
    }

    // 구분선(옵션)
    {
        auto line = new QFrame(content);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);
    }

    // 2) 친구 목록 카드들 (고정 높이)
    {
        DB db(this->window());
        QList<QVariantMap> rows;
        if (!db.listFriends(myId, &rows)) return;
        for (const auto& r : rows) {
            appendFriendCard(r["login_id"].toString(),
                             r["name"].toString(),
                             r["status"].toString());
        }
    }

    // 여백
    layout->addStretch(1);
}

QWidget* friendList::makeProfileCard(const QString& titleLine,
                                     const QString& subLine,
                                     bool emphasize,
                                     QWidget* parent)
{
    auto card = new QWidget(parent);
    card->setObjectName("profileCard");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setMinimumHeight(kCardHeight);
    card->setMaximumHeight(kCardHeight);

    // 스타일: 강조 카드(내 카드)는 옅은 배경
    card->setStyleSheet(emphasize
                            ? "#profileCard{ border:1px solid #dcdcdc; border-radius:8px; background:#f7f9fc; }"
                            : "#profileCard{ border:1px solid #e5e5e5; border-radius:8px; }");

    auto h = new QHBoxLayout(card);
    h->setContentsMargins(10,10,10,10);
    h->setSpacing(12);

    // 아바타(플레이스홀더 원형)
    auto avatar = new QLabel(card);
    avatar->setFixedSize(40,40);
    avatar->setStyleSheet("border-radius:20px; background:#e6ecf1;");
    h->addWidget(avatar);

    // 텍스트 영역
    auto v = new QVBoxLayout();
    v->setContentsMargins(0,0,0,0);
    v->setSpacing(2);

    auto nameLbl = new QLabel(titleLine, card);
    nameLbl->setStyleSheet("font-weight:600;");
    nameLbl->setWordWrap(false);
    nameLbl->setMinimumHeight(20);

    auto statusLbl = new QLabel(subLine.isEmpty()? tr("상태메시지 없음") : subLine, card);
    statusLbl->setStyleSheet("color:gray;");
    statusLbl->setWordWrap(false);
    statusLbl->setMinimumHeight(18);

    v->addWidget(nameLbl);
    v->addWidget(statusLbl);

    h->addLayout(v);
    h->addStretch(1);

    card->setLayout(h);
    return card;
}

static bool confirm1on1(QWidget* parent) {
    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("채팅하기"));
    dlg.setModal(true);
    dlg.setFixedSize(320, 200);

    auto v = new QVBoxLayout(&dlg);
    auto title = new QLabel(QObject::tr("💬  1:1 채팅하기"), &dlg);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:16px; font-weight:600; margin-top:12px;");
    v->addWidget(title);

    v->addStretch(1);

    auto btns = new QHBoxLayout();
    auto yes = new QPushButton(QObject::tr("예"), &dlg);
    auto no  = new QPushButton(QObject::tr("아니오"), &dlg);
    yes->setFixedSize(88,36);
    no->setFixedSize(88,36);
    btns->addStretch(1); btns->addWidget(yes); btns->addSpacing(12); btns->addWidget(no); btns->addStretch(1);
    v->addLayout(btns);
    v->addSpacing(12);

    QObject::connect(yes, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(no,  &QPushButton::clicked, &dlg, &QDialog::reject);
    return dlg.exec() == QDialog::Accepted;
}
