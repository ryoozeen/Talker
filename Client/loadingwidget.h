#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QWidget>
class QTimer;

namespace Ui { class LoadingWidget; }

class LoadingWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoadingWidget(QWidget *parent = nullptr);
    ~LoadingWidget();

signals:
    void finished();

public slots:
    void start();

private slots:
    void tick();

private:
    Ui::LoadingWidget *ui;
    QTimer *timer;
    int value;
};

#endif // LOADINGWIDGET_H
