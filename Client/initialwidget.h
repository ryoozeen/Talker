#ifndef INITIALWIDGET_H
#define INITIALWIDGET_H

#include <QWidget>

namespace Ui { class InitialWidget; }

class InitialWidget : public QWidget {
    Q_OBJECT
public:
    explicit InitialWidget(QWidget *parent = nullptr);
    ~InitialWidget();

signals:
    void startRequested();

private slots:
    void onLogoClicked(); // 로고 버튼 클릭 시 실행할 슬롯

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::InitialWidget *ui;
    void updateLogo();
};

#endif // INITIALWIDGET_H
