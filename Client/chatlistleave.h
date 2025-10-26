#ifndef CHATLISTLEAVE_H
#define CHATLISTLEAVE_H
#include <QWidget>
#include <QVariantMap>
class QListWidgetItem;
namespace Ui { class ChatListLeave; }

class ChatListLeave : public QWidget {
    Q_OBJECT

public:
    explicit ChatListLeave(QWidget* parent=nullptr);
    ~ChatListLeave();
    void reload(const QString& me);      // DB에서 현재 목록 로드

signals:
    void cancelRequested();              // 하단 취소 → 설정 화면으로
    void leftOne();                      // ✅ 신규: 한 방 나간 뒤 알림

public slots:    // ← slots 로 선언(아니면 public: 이어도 동작하지만 slots 권장)
    void onMessageActivity(qint64 roomId,
                           const QString& lastBody,
                           const QDateTime& ts);

private:
    void populate(const QList<QVariantMap>& rows);
    void bumpRoom(qint64 roomId, const QString& lastBody, const QDateTime& ts);

    Ui::ChatListLeave* ui;
    QString me_;
};
#endif
