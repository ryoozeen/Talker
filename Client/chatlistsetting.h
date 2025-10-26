#ifndef CHATLISTSETTING_H
#define CHATLISTSETTING_H

#include <QWidget>
namespace Ui { class ChatListSetting; }

class ChatListSetting : public QWidget {
    Q_OBJECT
public:
    explicit ChatListSetting(QWidget* parent=nullptr);
    ~ChatListSetting();

signals:
    void leaveModeRequested();   // “채팅방 나가기”
    void cancelRequested();      // “취소”

private:
    Ui::ChatListSetting* ui;
};
#endif
