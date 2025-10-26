#include "chatlistleave.h"
#include "ui_chatlistleave.h"
#include "chatlistitem.h"
#include "popup.h"
#include "db.h"
#include <QListWidget>
#include <QDateTime>

ChatListLeave::ChatListLeave(QWidget* parent) : QWidget(parent), ui(new Ui::ChatListLeave) {
    ui->setupUi(this);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &ChatListLeave::cancelRequested);
}

ChatListLeave::~ChatListLeave(){ delete ui; }

void ChatListLeave::reload(const QString& me) {
    me_ = me;
    // ↙︎ ChatList::reload에서 쓰던 쿼리와 동일하게 rows 채워주세요.
    // 예시: DB 유틸에 같은 SELECT가 이미 있다면 재사용, 없으면 아래처럼 직접 작성
    QList<QVariantMap> rows;
    {
        DB db;
        if (!db.connectTodb()) { Popup::error(this, tr("DB 연결 실패")); return; }
        // SELECT room list (dm_rooms + last message)
        rows = db.fetchChatList(me_); // ← 이미 있다면 이 한 줄로 끝
        // 없으면 ChatList 쿼리 그대로 복사해서 rows 채우기
    }
    populate(rows);
}

void ChatListLeave::populate(const QList<QVariantMap>& rows) {
    ui->leaveList->clear();
    for (const auto& r : rows) {
        const qint64 roomId   = r.value("room_id").toLongLong();
        const QString peerId  = r.value("peer_id").toString();
        const QString peerNm  = r.value("peer_name").toString();
        const QString lastBody= r.value("last_body").toString();
        const QDateTime lastTs= r.value("last_time").toDateTime();

        const QString display = peerNm.isEmpty() ? peerId : peerNm;
        const QString preview = lastBody.left(40);
        const QString whenStr = lastTs.isValid()? lastTs.toString("yyyy. MM. dd") : QString();

        auto* it = new QListWidgetItem(ui->leaveList);
        it->setSizeHint(QSize(0, 80));
        it->setData(Qt::UserRole+1, roomId);     // RoleRoomId와 값만 같게
        it->setData(Qt::UserRole+2, peerId);
        it->setData(Qt::UserRole+3, display);

        auto* w = new ChatListItem(ui->leaveList);
        w->setData(roomId, display, preview, whenStr);
        w->setLeaveMode(true); // 항상 빨간 버튼 보이게

        // 삭제 로직: DB 트랜잭션 + 새로고침
        connect(w, &ChatListItem::exitClicked, this, [this](qint64 rid, const QString& displayName){
            if (!Popup::confirmOkCancel(this,
                                        tr("'%1' 채팅방을 삭제하고 모든 메시지를 DB에서 지우시겠습니까?\n(되돌릴 수 없습니다)").arg(displayName),
                                        tr("채팅방 나가기"))) return;

            QString err;
            DB dbh;
            if (!dbh.leaveDmRoom(rid, me_, &err)) {                 // ✅ 내 쪽만 숨김+초기화
                Popup::error(this, tr("나가기 실패: %1").arg(err), tr("오류"));
                return;
            }
            Popup::success(this, tr("채팅방을 나갔습니다."), tr("완료"));

            // 리스트에서 즉시 제거(선택) — 화면에서 바로 사라지는 효과
            for (int i=0;i<ui->leaveList->count();++i) {
                auto *it = ui->leaveList->item(i);
                if (it && it->data(Qt::UserRole+1).toLongLong()==rid) {
                    delete ui->leaveList->takeItem(i);
                    break;
                }
            }
            emit leftOne();     // ✅ MainWindow가 이 신호를 받아서 "설정 화면"으로 전환
        });

        ui->leaveList->addItem(it);
        ui->leaveList->setItemWidget(it, w);
    }

    if (ui->leaveList->count() == 0)
        ui->leaveList->addItem(new QListWidgetItem(tr("채팅방이 없습니다.")));
}

void ChatListLeave::bumpRoom(qint64 roomId, const QString& lastBody, const QDateTime& ts) {
    int idx = -1;
    for (int i = 0; i < ui->leaveList->count(); ++i) {
        auto* it = ui->leaveList->item(i);
        if (it && it->data(Qt::UserRole + 1).toLongLong() == roomId) { idx = i; break; }
    }
    if (idx < 0) { reload(me_); return; }

    auto* it = ui->leaveList->item(idx);
    const QString display = it->data(Qt::UserRole + 3).toString();
    const QString preview = lastBody.left(40);
    const QString dateStr = ts.isValid() ? ts.toString("yyyy. MM. dd") : QString();

    // 기존 위젯 확보
    QWidget* oldWidget = ui->leaveList->itemWidget(it);

    // 아이템 이동
    auto* taken = ui->leaveList->takeItem(idx);
    ui->leaveList->insertItem(0, taken);
    taken->setSizeHint(QSize(0, 80));

    // 항상 ChatListItem 사용 (나가기 화면)
    ChatListItem* w = qobject_cast<ChatListItem*>(oldWidget);
    if (!w) w = new ChatListItem(ui->leaveList);
    w->setData(roomId, display, preview, dateStr);
    w->setLeaveMode(true);
    ui->leaveList->setItemWidget(taken, w);

    ui->leaveList->clearSelection();
}

void ChatListLeave::onMessageActivity(qint64 roomId,
                                      const QString& lastBody,
                                      const QDateTime& ts)
{
    bumpRoom(roomId, lastBody, ts);
}
