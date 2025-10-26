#include "profilechange.h"
#include "ui_profilechange.h"
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

ProfileChange::ProfileChange(QWidget *parent)
    : QWidget(parent), ui(new Ui::ProfileChange)
{
    ui->setupUi(this);

    // 한글/이모지 포함 255자 제한
    if (auto e = this->findChild<QLineEdit*>("statusEdit")) {
        e->setMaxLength(255);
    }

    // 버튼 연결
    connect(ui->cancelBtn, &QPushButton::clicked,
            this,           &ProfileChange::cancelRequested);

    if (auto a = this->findChild<QPushButton*>("applyBtn")) {
        connect(a, &QPushButton::clicked, this, [this]{
            auto e = this->findChild<QLineEdit*>("statusEdit");
            const QString text = e ? e->text().trimmed().left(255) : QString();
            emit applyRequested(text);
        });
    }
}

ProfileChange::~ProfileChange() { delete ui; }

void ProfileChange::setInitial(const QString& meName, const QString& currentStatus)
{
    if (auto m = this->findChild<QLabel*>("meLabel")) {
        m->setText(meName.isEmpty() ? tr("나") : meName);
    }
    if (auto e = this->findChild<QLineEdit*>("statusEdit")) {
        e->setText(currentStatus);
        e->setCursorPosition(e->text().size());
        e->setFocus();
    }
}
