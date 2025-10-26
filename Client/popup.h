#ifndef POPUP_H
#define POPUP_H

#include <QString>
#include <QMessageBox>

namespace Popup {

QMessageBox* makeBox(QWidget* parent,
                     QMessageBox::Icon icon,
                     const QString& text,
                     const QString& title,
                     int width);

void info(QWidget* parent, const QString& text,
          const QString& title = QString(), int width = 0);
void warning(QWidget* parent, const QString& text,
             const QString& title = QString(), int width = 0);
void error(QWidget* parent, const QString& text,
           const QString& title = QString(), int width = 0);
bool confirmYesNo(QWidget* parent, const QString& text,
                  const QString& title = QString(), int width = 0);
bool confirmOkCancel(QWidget* parent, const QString& text,
                     const QString& title = QString(), int width = 0);
void success(QWidget* parent, const QString& text,
             const QString& title = QString(), int width = 0);
}

#endif // POPUP_H
