#include "chatlist.h"
#include "chatlistitem.h"

#include "popup.h"
#include "db.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QVariantMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>

enum {
    RoleRoomId = Qt::UserRole + 1,
    RolePeerId,
    RolePeerName
};

ChatList::ChatList(QWidget *parent) : QWidget(parent) {
    buildUi();
}

void ChatList::buildUi() {
    auto v = new QVBoxLayout(this);
    v->setContentsMargins(10,10,10,10);
    v->setSpacing(8);

    // ── 상단 헤더
    auto header = new QHBoxLayout();

    auto title = new QLabel(tr("채팅방 목록"), this);
    title->setStyleSheet("font-size:18px; font-weight:700;");

    auto gear = new QPushButton("⚙️", this);     // (옵션) 우측 설정 아이콘
    gear->setFlat(true);
    gear->setFixedSize(50,50);

    header->addStretch(1);
    header->addWidget(title, 0, Qt::AlignCenter);
    header->addStretch(1);
    header->addWidget(gear, 0, Qt::AlignRight);
    v->addLayout(header);

    connect(gear, &QPushButton::clicked, this, [this]{ emit openSettingRequested(); });

    // ── 목록
    roomList_ = new QListWidget(this);
    roomList_->setObjectName("roomList");
    roomList_->setUniformItemSizes(true);
    roomList_->setSelectionMode(QAbstractItemView::SingleSelection);
    roomList_->setStyleSheet(R"(
        QListWidget#roomList {
            border: 1px solid #e5e5e5; border-radius: 6px; background: #fff;
        }
        QListWidget#roomList::item {
            margin: 6px;               /* 카드 사이 여백 */
        }
        /* 선택/포커스시 카드 배경 강조 */
        QListWidget#roomList::item:selected {
            background: transparent;
        }
    )");
    v->addWidget(roomList_, 1);


    // ── 하단 탭 (친구/채팅/더보기) — 친구목록과 동일한 느낌
    auto tabs = new QHBoxLayout();
    tabFriend_ = new QPushButton(tr("🗣️\n친구"), this);
    tabChat_   = new QPushButton(tr("🗨️\n채팅"), this);
    tabMore_   = new QPushButton(tr("🔵\n더보기"), this);
    for (auto* b : {tabFriend_, tabChat_, tabMore_}) {
        b->setMinimumHeight(44);
        b->setStyleSheet("QPushButton{border:1px solid #ddd; border-radius:8px; background:#fff;}");
    }
    tabs->addWidget(tabFriend_);
    tabs->addWidget(tabChat_);
    tabs->addWidget(tabMore_);
    v->addLayout(tabs);

    // ── 시그널
    connect(backBtn_, &QPushButton::clicked, this, &ChatList::backRequested);
    connect(roomList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* it){
        if (!it) return;
        emit openRoomRequested(
            it->data(RoleRoomId).toLongLong(),
            it->data(RolePeerId).toString(),
            it->data(RolePeerName).toString()
            );
    });
    connect(tabFriend_, &QPushButton::clicked, this, &ChatList::friendTabRequested);
    connect(tabChat_,   &QPushButton::clicked, this, &ChatList::chatTabRequested);
    connect(tabMore_,   &QPushButton::clicked, this, &ChatList::moreTabRequested);
}

void ChatList::reload(const QString& me)
{
    me_ = me;
    roomList_->clear();

    QString err;
    // DB 유틸이 user_room_state(숨김/cleared_at)까지 반영해서 목록을 만들어줍니다.
    QList<QVariantMap> rows = DB(this).fetchChatList(me_, &err);
    if (!err.isEmpty()) {
        showError(tr("목록을 불러오는 중 오류: %1").arg(err));
        return;
    }
    populate(rows);
}

QWidget* ChatList::makeRow(const QString& displayName,
                           const QString& lastBody,
                           const QDateTime& lastTs,
                           QWidget* parent)
{
    // 카드 프레임
    auto card = new QWidget(parent);
    card->setObjectName("roomCard");
    card->setMinimumHeight(68);
    card->setStyleSheet(R"(
        #roomCard { border:1px solid #e5e5e5; border-radius:8px; background:#fff; }
        #nameLbl  { font-size:16px; font-weight:700; color:#111; }
        #bodyLbl  { font-size:13px; color:#666; }
        #timeLbl  { font-size:12px; color:#999; }
    )");

    auto h = new QHBoxLayout(card);
    h->setContentsMargins(12,10,12,10);
    h->setSpacing(8);

    // 좌측: 이름(굵게, 크게) + 본문(바로 아래 줄, 좌측)
    auto v = new QVBoxLayout();
    auto nameLbl = new QLabel(displayName, card);
    nameLbl->setObjectName("nameLbl");
    nameLbl->setMinimumHeight(20);

    auto bodyLbl = new QLabel(lastBody.isEmpty() ? tr("(메시지 없음)") : lastBody, card);
    bodyLbl->setObjectName("bodyLbl");
    bodyLbl->setWordWrap(false);
    bodyLbl->setMinimumHeight(18);
    v->addWidget(nameLbl);
    v->addWidget(bodyLbl);

    // 우측: 시간 (맨 오른쪽 정렬)
    auto timeLbl = new QLabel(lastTs.isValid() ? lastTs.toString("yyyy-MM-dd HH:mm") : "", card);
    timeLbl->setObjectName("timeLbl");

    h->addLayout(v, 1);
    h->addWidget(timeLbl, 0, Qt::AlignRight | Qt::AlignTop);

    card->setLayout(h);
    return card;
}

void ChatList::populate(const QList<QVariantMap>& rows) {
    roomList_->clear();

    for (const auto& r : rows) {
        const qint64 roomId   = r.value("room_id").toLongLong();
        const QString peerId  = r.value("peer_id").toString();
        const QString peerNm  = r.value("peer_name").toString();
        const QString lastBody= r.value("last_body").toString();
        const QDateTime lastTs= r.value("last_time").toDateTime();

        const QString display = peerNm.isEmpty() ? peerId : peerNm;
        const QString preview = lastBody.left(40);                // 한 줄 미리보기
        const QString lastDateString = lastTs.isValid()
                                           ? lastTs.toString("yyyy. MM. dd")
                                           : QString();

        auto* it = new QListWidgetItem(roomList_);
        it->setData(RoleRoomId,   roomId);
        it->setData(RolePeerId,   peerId);
        it->setData(RolePeerName, display);
        it->setSizeHint(QSize(0, 80));                           // 카드 높이
        roomList_->addItem(it);

        if (!leaveMode_) {
            QWidget* card = makeRow(display, preview, lastTs, roomList_);
            roomList_->setItemWidget(it, card);
        } else {
            auto* w = new ChatListItem(roomList_);
            w->setData(roomId, display, preview, lastTs.isValid()? lastTs.toString("yyyy. MM. dd") : QString());
            w->setLeaveMode(true);

            connect(w, &ChatListItem::exitClicked, this, [this](qint64 rid, const QString& displayName){
                if (!Popup::confirmOkCancel(this,
                                            tr("'%1' 채팅방을 삭제하고 모든 메시지를 DB에서 지우시겠습니까?\n(되돌릴 수 없습니다)").arg(displayName),
                                            tr("채팅방 나가기"))) return;

                QString err;
                DB dbh;
                if (!dbh.leaveDmRoom(rid, me_, &err)) {
                    Popup::error(this, tr("나가기 실패: %1").arg(err), tr("오류"));
                    return;
                }
                Popup::success(this, tr("채팅방을 나갔습니다. 내 목록/기록에서만 제거됩니다."), tr("완료"));
                reload(me_);          // 내 목록에서만 제거
            });

            roomList_->setItemWidget(it, w);
        }
    }

    if (roomList_->count() == 0) {
        roomList_->addItem(new QListWidgetItem(tr("채팅방이 없습니다.")));
    }
}

void ChatList::showError(const QString& msg)
{
    roomList_->clear();
    auto* it = new QListWidgetItem(QString("목록을 불러오는 중 오류: %1").arg(msg));
    it->setBackground(QColor("#e45740"));
    it->setForeground(Qt::white);
    roomList_->addItem(it);
}

void ChatList::setLeaveMode(bool on) {
    leaveMode_ = on;
    reload(me_);
}

void ChatList::onMessageActivity(qint64 roomId, const QString& lastBody, const QDateTime& ts) {
    bumpRoom(roomId, lastBody, ts);
}

void ChatList::bumpRoom(qint64 roomId, const QString& lastBody, const QDateTime& ts) {
    // 1) 해당 아이템 인덱스 찾기
    int idx = -1;
    for (int i = 0; i < roomList_->count(); ++i) {
        auto* it = roomList_->item(i);
        if (it && it->data(RoleRoomId).toLongLong() == roomId) { idx = i; break; }
    }
    if (idx < 0) {
        // 목록에 없으면 전체 리로드 (최초 대화 등)
        reload(me_);
        return;
    }

    auto* it = roomList_->item(idx);

    // 표시할 텍스트들 준비
    const QString display = it->data(RolePeerName).toString();
    const QString preview = lastBody.left(40);
    const QString dateStr = ts.isValid() ? ts.toString("yyyy. MM. dd") : QString();

    // 2) 행을 위로 올리기 전에, 기존에 붙어 있던 위젯 포인터를 확보
    QWidget* oldWidget = roomList_->itemWidget(it);

    // 3) 아이템을 꺼내 맨 위로 이동
    auto* taken = roomList_->takeItem(idx);
    roomList_->insertItem(0, taken);
    taken->setSizeHint(QSize(0, 80));  // 카드 높이 유지

    // 4) 모드에 따라 "새 위젯"을 만들어 'taken'에 다시 붙인다
    if (!leaveMode_) {
        // 일반 모드: 기존 makeRow 카드 새로 만들어 붙임
        QWidget* card = makeRow(display, preview, ts, roomList_);
        roomList_->setItemWidget(taken, card);

        // oldWidget은 더 이상 쓰지 않으므로 정리(메모리 누수 방지)
        if (oldWidget && oldWidget != card) oldWidget->deleteLater();
    } else {
        // 나가기 모드: ChatListItem(ui) 사용
        ChatListItem* w = qobject_cast<ChatListItem*>(oldWidget);
        if (!w) w = new ChatListItem(roomList_);
        w->setData(roomId, display, preview, dateStr);
        w->setLeaveMode(true);
        roomList_->setItemWidget(taken, w);
    }

    // 5) 선택 상태/스크롤 보정
    roomList_->clearSelection();
}
