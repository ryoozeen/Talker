#include "popup.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QPushButton>

namespace Popup {

QMessageBox* makeBox(QWidget* parent,
                     QMessageBox::Icon icon,
                     const QString& text,
                     const QString& title,
                     int width)
{
    auto* box = new QMessageBox(parent);
    box->setIcon(icon);
    box->setText(text);
    if (!title.isEmpty())
        box->setWindowTitle(title);
    if (width > 0) {
        box->setMinimumWidth(width);
        box->setStyleSheet(QStringLiteral("QLabel{min-width:%1px;}").arg(width));
    }
    box->setWindowModality(Qt::ApplicationModal);

    // === 위치 조정: 활성 창 → 부모 최상위 창 → 기본 스크린 중앙 ===
    box->ensurePolished();
    box->adjustSize();

    QWidget* anchor = QApplication::activeWindow();
    if (!anchor && parent) anchor = parent->window();

    if (anchor) {
        const QRect fr = anchor->frameGeometry();
        const QSize sz = box->sizeHint();
        const int x = fr.x() + (fr.width()  - sz.width())  / 2;
        const int y = fr.y() + (fr.height() - sz.height()) / 2;
        box->move(x, y);
    } else if (QScreen* sc = QGuiApplication::primaryScreen()) {
        const QRect ar = sc->availableGeometry();
        const QSize sz = box->sizeHint();
        const int x = ar.x() + (ar.width()  - sz.width())  / 2;
        const int y = ar.y() + (ar.height() - sz.height()) / 2;
        box->move(x, y);
    }

    return box;
}

void info(QWidget* parent, const QString& text,
          const QString& title, int width) {
    auto box = makeBox(parent, QMessageBox::Information, text, title, width);
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText(QObject::tr("확인"));
    box->exec();
    delete box;
}

void warning(QWidget* parent, const QString& text,
             const QString& title, int width) {
    auto box = makeBox(parent, QMessageBox::Warning, text, title, width);
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText(QObject::tr("확인"));
    box->exec();
    delete box;
}

void error(QWidget* parent, const QString& text,
           const QString& title, int width) {
    auto box = makeBox(parent, QMessageBox::Critical, text, title, width);
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText(QObject::tr("확인"));
    box->exec();
    delete box;
}

bool confirmYesNo(QWidget* parent, const QString& text,
                  const QString& title, int width) {
    auto box = makeBox(parent, QMessageBox::Question, text, title, width);
    box->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box->button(QMessageBox::Yes)->setText(QObject::tr("예"));
    box->button(QMessageBox::No)->setText(QObject::tr("아니오"));
    box->setDefaultButton(QMessageBox::No);
    const auto ret = box->exec();
    delete box;
    return (ret == QMessageBox::Yes);
}

bool confirmOkCancel(QWidget* parent, const QString& text,
                     const QString& title, int width) {
    auto box = makeBox(parent, QMessageBox::Question, text, title, width);
    box->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    box->button(QMessageBox::Ok)->setText(QObject::tr("확인"));
    box->button(QMessageBox::Cancel)->setText(QObject::tr("취소"));
    box->setDefaultButton(QMessageBox::Cancel);
    const auto ret = box->exec();
    delete box;
    return (ret == QMessageBox::Ok);
}

// 성공(Information 아이콘 사용)
void success(QWidget* parent, const QString& text,
             const QString& title, int width) {
    auto box = makeBox(parent, QMessageBox::Information, text, title, width);
    box->setStandardButtons(QMessageBox::Ok);
    box->button(QMessageBox::Ok)->setText(QObject::tr("확인"));
    box->exec();
    delete box;
}

} // namespace Popup
