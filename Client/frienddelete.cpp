#include "frienddelete.h"
#include "ui_frienddelete.h"

#include "db.h"
#include "popup.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

static constexpr int kRowH = 70;

FriendDelete::FriendDelete(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendDelete)
{
    ui->setupUi(this);

    // UI 포인터 캐시
    area_ = ui->scrollArea;
    // scrollArea->widget()가 listContainer이고, 그 안의 레이아웃이 listLayout
    listLayout_ = qobject_cast<QVBoxLayout*>(ui->listContainer->layout());

    // 취소 버튼
    connect(ui->cancelBtn, &QPushButton::clicked, this, &FriendDelete::back);
}

FriendDelete::~FriendDelete()
{
    delete ui;
}

void FriendDelete::setOwnerId(const QString& ownerId)
{
    ownerId_ = ownerId.trimmed();
    reload();
}

void FriendDelete::reload()
{
    if (!listLayout_) return;

    // 기존 줄 비우기
    while (QLayoutItem* it = listLayout_->takeAt(0)) {
        if (auto w = it->widget()) w->deleteLater();
        delete it;
    }

    if (ownerId_.isEmpty()) return;

    // DB에서 친구 목록 로딩
    DB db(this);
    QList<QVariantMap> rows;
    if (!db.listFriends(ownerId_, &rows)) return;

    for (const auto& r : rows) {
        auto row = makeRow(
            r.value("login_id").toString(),
            r.value("name").toString(),
            r.value("status").toString()
        );
        listLayout_->addWidget(row);
    }
    listLayout_->addStretch(1);
}

QWidget* FriendDelete::makeRow(const QString& fid,
                               const QString& name,
                               const QString& status)
{
    auto row = new QWidget(this);
    row->setObjectName(QStringLiteral("delRow"));
    row->setStyleSheet("#delRow{ border:1px solid #ddd; border-radius:8px; }");
    row->setMinimumHeight(kRowH);
    row->setMaximumHeight(kRowH);
    row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // ⬅️ 추가

    auto h = new QHBoxLayout(row);
    h->setContentsMargins(10,8,10,8);
    h->setSpacing(10);

    // 좌측 텍스트
    auto v = new QVBoxLayout();

    auto nameLbl = new QLabel(QString("%1 (%2)").arg(name, fid), row);
    nameLbl->setStyleSheet("font-weight:600;");

    auto statusLbl = new QLabel(status.isEmpty()? tr("상태메시지") : status, row);
    statusLbl->setStyleSheet("color:gray;");
    statusLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    statusLbl->setWordWrap(false);

    v->addWidget(nameLbl);
    v->addWidget(statusLbl);

    h->addLayout(v);
    h->addStretch(1);

    // 우측 빨간 삭제 버튼
    auto del = new QPushButton(tr("삭제"), row);
    del->setFixedSize(72,36);
    del->setStyleSheet("QPushButton{background:#d90000; color:white; border:0; border-radius:6px;}");
    h->addWidget(del);

    // 삭제 처리
    connect(del, &QPushButton::clicked, this, [this, fid]{
        if (ownerId_.isEmpty()) return;
        DB db(this);
        QString err;
        if (!db.removeFriend(ownerId_, fid, &err)) {
            Popup::error(this, tr("친구 삭제 실패: %1").arg(err), tr("오류"));
            return;
        }
        Popup::success(this, tr("삭제했습니다."), tr("완료"));
        emit deletedOne();  // 메인에서 친구목록 최신화
        // reload();           // 현재 화면 리스트 즉시 갱신
    });

    row->setLayout(h);
    return row;
}
