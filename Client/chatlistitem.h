// chatlistitem.h
#ifndef CHATLISTITEM_H
#define CHATLISTITEM_H
#include <QWidget>
namespace Ui { class ChatListItem; }

class ChatListItem : public QWidget {
    Q_OBJECT
public:
    explicit ChatListItem(QWidget* parent=nullptr);
    ~ChatListItem();
    void setData(qint64 roomId, const QString& name, const QString& preview, const QString& date);
    void setLeaveMode(bool on);
    qint64 roomId() const { return roomId_; }

signals:
    void exitClicked(qint64 roomId, const QString& displayName);

private:
    Ui::ChatListItem* ui;
    qint64 roomId_ = 0;
};
#endif
