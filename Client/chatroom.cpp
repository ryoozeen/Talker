#include "chatroom.h"
#include "db.h"

#include <QSqlQuery>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QScrollBar>
#include <QFileDialog>
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QDir>

ChatRoom::ChatRoom(QWidget *parent) : QWidget(parent) { buildUi(); }

void ChatRoom::buildUi() {
    auto v = new QVBoxLayout(this);
    v->setContentsMargins(10,10,10,10);
    v->setSpacing(8);


    // ── 상단 헤더 (뒤로 / 제목 / 검색 / 설정)
    auto header = new QHBoxLayout();
    backBtn_   = new QPushButton(QString::fromUtf8("◀"), this);
    searchBtn_ = new QPushButton(QString::fromUtf8("🔍"), this);
    settingBtn_ = new QPushButton(QString::fromUtf8("⚙"), this);

    for (auto* b : {backBtn_, searchBtn_, settingBtn_}) {
        b->setFixedSize(40, 40);
        b->setCursor(Qt::PointingHandCursor);
            b->setStyleSheet("QPushButton{border:0; background:transparent; font-size:20px; font-weight:700;}");
    }

    title_ = new QLabel(tr("채팅방"), this);
    title_->setAlignment(Qt::AlignCenter);
    title_->setStyleSheet("font-size:20px; font-weight:700;");

    header->addWidget(backBtn_, 0, Qt::AlignLeft);
    header->addStretch(1);
    header->addWidget(title_, 0, Qt::AlignCenter);
    header->addStretch(1);
    header->addWidget(searchBtn_, 0, Qt::AlignRight);
    header->addWidget(settingBtn_, 0, Qt::AlignRight);
    v->addLayout(header);

    // ── 대화 영역
    chatView_ = new QTextBrowser(this);
    chatView_->setOpenExternalLinks(true);
    chatView_->setReadOnly(true);
    chatView_->setMinimumHeight(300);
    chatView_->setStyleSheet("QTextBrowser{border:1px solid #e5e5e5; border-radius:6px; background:#fff;}");
    v->addWidget(chatView_, 1);

    // ── 입력줄 ( +  /  입력  /  전송 )
    auto inputRow = new QHBoxLayout();
    plusBtn_ = new QPushButton("+", this);
    plusBtn_->setFixedSize(36,36);
    plusBtn_->setCursor(Qt::PointingHandCursor);
    plusBtn_->setStyleSheet("QPushButton{font-size:20px; border:1px solid #ddd; border-radius:6px; background:#fff;}");

    input_ = new QLineEdit(this);
    input_->setPlaceholderText(tr("메시지 입력"));
    input_->setMinimumHeight(36);
    input_->setStyleSheet("QLineEdit{border:1px solid #ddd; border-radius:6px; padding:8px;}");

    sendBtn_ = new QPushButton(tr("전송"), this);
    sendBtn_->setFixedHeight(36);
    sendBtn_->setCursor(Qt::PointingHandCursor);
    sendBtn_->setStyleSheet("QPushButton{border:1px solid #0b7; color:#0b7; background:#fff; border-radius:6px; padding:0 12px;}");

    inputRow->addWidget(plusBtn_, 0);
    inputRow->addSpacing(6);
    inputRow->addWidget(input_, 1);
    inputRow->addSpacing(6);
    inputRow->addWidget(sendBtn_, 0);
    v->addLayout(inputRow);

    // ── 시그널
    connect(backBtn_, &QPushButton::clicked, this, [this]{
        // 현재 화면을 캐시해두면 다시 들어왔을 때 그대로 보임
        htmlCache_[roomId_] = chatView_->toHtml();
        emit backRequested();
    });
    connect(sendBtn_, &QPushButton::clicked, this, &ChatRoom::onSend);
    connect(input_,    &QLineEdit::returnPressed, this, &ChatRoom::onSend);
    connect(plusBtn_, &QPushButton::clicked, this, &ChatRoom::onFilePlus);

    chatView_->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
}

void ChatRoom::setSocket(QTcpSocket* s) {
    if (socket_) socket_->disconnect(this);
    socket_ = s;
    // if (socket_) connect(socket_, &QTcpSocket::readyRead, this, &ChatRoom::onReadyRead);
}

void ChatRoom::open(qint64 roomId, const QString& me,
                    const QString& peerId, const QString& peerName)
{
    roomId_ = roomId;  myLoginId_ = me;  peerId_ = peerId;  peerName_ = peerName;
    title_->setText(peerName_.isEmpty()? peerId_ : peerName_);

    // 1) 세션 캐시가 있으면 먼저 보여주고
    if (htmlCache_.contains(roomId_)) {
        chatView_->setHtml(htmlCache_.value(roomId_));
    } else {
        chatView_->clear();
    }
    // 2) 항상 DB에서 새로 로드해 진실값으로 갱신
    loadHistory();

    chatView_->verticalScrollBar()->setValue(chatView_->verticalScrollBar()->maximum());
    input_->setFocus();

    // (선택) 입장 알림 송신: 서버가 messages에 적고 브로드캐스트
    // QJsonObject obj{
        // {"type","enter"},
        // {"room_id", static_cast<double>(roomId_)},
        // {"sender", myLoginId_},
        // {"text", [this]{
        // QString err, myName = DB(this).getUserName(myLoginId_, &err);
           // if (myName.isEmpty()) myName = tr("나");
           // return tr("%1(%2)님이 입장하셨습니다.").arg(myName, myLoginId_);
         // }()}
    // };
    // sendJson(obj);
}

void ChatRoom::loadHistory() {
    QString err;
    if (!DB::ensureConnection(&err)) return;
    QSqlDatabase h = DB::handle();

    // 내 cleared_at 조회
    QDateTime ca;
    DB db;
    db.getClearedAt(roomId_, myLoginId_, &ca, nullptr);

    QSqlQuery q(h);
    if (ca.isValid()) {
        q.prepare("SELECT sender_login_id, body, sent_at "
                  "FROM messages WHERE room_id=? AND sent_at>? "
                  "ORDER BY sent_at ASC LIMIT 1000");
        q.addBindValue(roomId_);
        q.addBindValue(ca);
    } else {
        q.prepare("SELECT sender_login_id, body, sent_at "
                  "FROM messages WHERE room_id=? ORDER BY sent_at ASC LIMIT 1000");
        q.addBindValue(roomId_);
    }
    if (!q.exec()) return;

    chatView_->clear();
    while (q.next()) {
        const auto who = q.value(0).toString();
        const auto txt = q.value(1).toString();
        const auto ts  = q.value(2).toDateTime().toString("yyyy-MM-dd HH:mm");
        appendBubble(who, txt, ts);
    }
    htmlCache_[roomId_] = chatView_->toHtml();
}

void ChatRoom::appendBubble(const QString& who, const QString& text, const QString& ts)
{
    const bool me = (who == myLoginId_);
    const QString whoName = me ? tr("나") : (peerName_.isEmpty()? peerId_ : peerName_);
    const QString tsStr = ts.isEmpty()
                              ? QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")
                              : ts;

    // 1) 문단 자체를 좌/우로 정렬
    QTextCursor cur = chatView_->textCursor();
    cur.movePosition(QTextCursor::End);
    QTextBlockFormat bf; bf.setAlignment(me ? Qt::AlignRight : Qt::AlignLeft);
    bf.setTopMargin(6); bf.setBottomMargin(6);
    cur.insertBlock(bf);

    // 2) 두 번째 줄 구성 (왼쪽: 메시지 · 시간 / 오른쪽: 시간 · 메시지)
    const QString secondRow = me
                                  // 나
                                  ? QString("<div style='text-align:right;'>"
                                            "<span style='font-size:11px;color:#888;white-space:nowrap;'>%1</span>"         // >> 이름 사이즈
                                            "<span style='display:inline-block;width:20px;'>　　　　　　　</span>"
                                            "<span style='font-size:14px;line-height:1.35;'>%2</span>"
                                            "</div>")
                                        .arg(tsStr.toHtmlEscaped(), text.toHtmlEscaped())

                                  // 상대방
                                  : QString("<div style='text-align:left;'>"
                                            "<span style='font-size:14px;line-height:1.35;'>%1</span>"
                                            "<span style='display:inline-block;width:20px;'>　　　　　　　</span>"
                                            "<span style='font-size:11px;color:#888;white-space:nowrap;'>%2</span>"
                                            "</div>")
                                        .arg(text.toHtmlEscaped(), tsStr.toHtmlEscaped());

    // 3) 버블(내 메시지일 땐 버블 내부도 우측 정렬)
    const QString bubble =
        QString("<div style='display:inline-block; max-width:72%%; "
                "border:1px solid %1; background:%2; border-radius:10px; padding:8px 10px; "
                "text-align:%3;'>"
                "<div style='font-size:12px;font-weight:700;color:#444; margin-bottom:2px;'>%4</div>"           // 이름 사이즈
                "%5"
                "</div>")
            .arg(me ? "#cfe8b6" : "#dddddd")
            .arg(me ? "#DCF8C6" : "#ffffff")
            .arg(me ? "right" : "left")
            .arg(whoName.toHtmlEscaped())
            .arg(secondRow);

    cur.insertHtml(bubble);

    // 4) 캐시 & 스크롤 맨 아래
    htmlCache_[roomId_] = chatView_->toHtml();
    if (auto *vs = chatView_->verticalScrollBar()) vs->setValue(vs->maximum());
}

void ChatRoom::sendJson(const QJsonObject& obj) {
    if (!socket_) return;
    auto line = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
    socket_->write(line);
}

void ChatRoom::onSend() {
    const auto text = input_->text().trimmed();
    if (text.isEmpty() || !socket_) return;

    // UI 즉시 반영
    appendBubble(myLoginId_, text, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
    input_->clear();

    // 내가 나갔던 방이면 내 쪽 숨김 해제
    {
        DB db;
        db.unhideDmRoom(roomId_, myLoginId_, nullptr);
    }

    // 서버에 전송 → 서버가 DB 저장 + 상대에게 브로드캐스트
    QJsonObject obj{
        {"type","chat"},
        {"room_id", static_cast<double>(roomId_)},
        {"sender", myLoginId_},
        {"text", text}
    };

    // 🔽🔽🔽 여기 한 줄 추가 (방ID: roomId_, 본문: text, 시간: now)
    emit messageActivity(roomId_, text, QDateTime::currentDateTime());

    sendJson(obj);
}

void ChatRoom::onReadyRead() {
    if (!socket_) return;
    while (socket_->canReadLine()) {
        const auto line = socket_->readLine();
        const auto obj = QJsonDocument::fromJson(line).object();
        const auto type = obj.value("type").toString();
        const auto rid  = static_cast<qint64>(obj.value("room_id").toDouble());
        if (rid != roomId_) continue;

        if (type == "chat") {
            const auto who = obj.value("sender").toString();
            const auto txt = obj.value("text").toString();
            const auto ts  = obj.value("ts").toString();   // 서버가 ISO-time 넣어줌
            appendBubble(who, txt, ts);
        }
        else if (type == "file") {
            const auto who = obj.value("sender").toString();
            const auto fname = obj.value("filename").toString();
            const auto mime  = obj.value("mime").toString();
            const auto b64   = obj.value("data").toString();
            const auto ts    = obj.value("ts").toString();
            const QByteArray bytes = QByteArray::fromBase64(b64.toLatin1());

            // 임시폴더에 저장 후, 이미지면 미리보기, 아니면 링크
            const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/talker_downloads";
            QDir().mkpath(baseDir);
            QString safeName = fname.isEmpty()? QString::number(QDateTime::currentMSecsSinceEpoch()) : fname;
            const QString path = baseDir + "/" + safeName;
            QFile f(path);
            if (f.open(QIODevice::WriteOnly)) { f.write(bytes); f.close(); }

            if (mime.startsWith("image/")) {
                const QString html = QString("<img src=\"file:///%1\" style='max-width:220px; max-height:220px;'>").arg(path);
                appendBubble(who, html, ts);
            } else {
                const QString link = QString("<a href='file:///%1'>%2</a>").arg(path, QFileInfo(path).fileName());
                appendBubble(who, link, ts);
            }
        }
    }
}

void ChatRoom::onFilePlus() {
    QFileDialog dlg(this, tr("파일 선택"));
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilters({tr("이미지 (*.png *.jpg *.jpeg *.gif)"),
                        tr("동영상 (*.mp4 *.mov *.avi)"),
                        tr("모든 파일 (*)")});
    if (dlg.exec() != QDialog::Accepted) return;

    const QStringList files = dlg.selectedFiles();
    QMimeDatabase md;
    constexpr qint64 kMax = 5 * 1024 * 1024; // 5MB

    for (const QString& path : files) {
        QFileInfo fi(path);
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) continue;
        const QByteArray bytes = f.read(kMax + 1);
        f.close();
        if (bytes.isEmpty() || bytes.size() > kMax) {
            appendBubble(myLoginId_, tr("파일이 너무 큽니다 (최대 5MB): %1").arg(fi.fileName()));
            continue;
        }

        const QString mime = md.mimeTypeForFile(fi).name();
        const QString b64  = QString::fromLatin1(bytes.toBase64());

        // UI 즉시 반영(미리보기/링크)
        if (mime.startsWith("image/")) {
            const QString html = QString("<img src=\"file:///%1\" style='max-width:220px; max-height:220px;'>").arg(path);
            appendBubble(myLoginId_, html, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
        } else {
            const QString link = QString("<a href='file:///%1'>%2</a>").arg(path, fi.fileName());
            appendBubble(myLoginId_, link, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));
        }

        // 서버에 전송
        QJsonObject obj{{"type","file"},
                        {"room_id", static_cast<double>(roomId_)},
                        {"filename", fi.fileName()},
                        {"mime", mime},
                        {"data", b64}};
        sendJson(obj);
        emit messageActivity(roomId_, tr("[파일] %1").arg(fi.fileName()), QDateTime::currentDateTime());
    }
}

void ChatRoom::handleServerMessage(const QJsonObject& obj) {
    const auto type = obj.value("type").toString();
    const auto rid  = static_cast<qint64>(obj.value("room_id").toDouble());
    if (rid != roomId_) return;

    if (type == "chat") {
        const auto who = obj.value("sender").toString();
        const auto txt = obj.value("text").toString();
        const auto ts  = obj.value("ts").toString();
        appendBubble(who, txt, ts);

        QDateTime when = QDateTime::fromString(ts, Qt::ISODate);
        if (!when.isValid()) when = QDateTime::currentDateTime();
        emit messageActivity(roomId_, txt, when);
    }
}
