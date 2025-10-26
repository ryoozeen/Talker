#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class pwReset; }
QT_END_NAMESPACE

class pwReset : public QWidget {   // ✅ QDialog → QWidget
    Q_OBJECT

public:
    explicit pwReset(QWidget *parent = nullptr);
    ~pwReset();

    void setLoginId(const QString& id);  // pwFind에서 넘겨받을 ID

signals:
    void done();  // 비밀번호 변경 완료 시 MainWindow로 알림

private:
    Ui::pwReset *ui;
    QString loginId_;
};
