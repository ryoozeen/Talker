#ifndef SERCHOK_H
#define SERCHOK_H


#include <QWidget>

namespace Ui {
class SerchOk;
}

class SerchOk : public QWidget
{
    Q_OBJECT

public:
    explicit SerchOk(QWidget *parent = nullptr);
    ~SerchOk();

    void setProfile(const QString& friendId, const QString& name, const QString& status);

signals:
    void back();
    void addFriendRequested(const QString& friendId);


private:
    Ui::SerchOk *ui;
    QString friendId_;
};

#endif // SERCHOK_H
