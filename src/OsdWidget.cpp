#include "OsdWidget.h"

#include <QCursor>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QScreen>
#include <QSvgWidget>

OsdWidget::OsdWidget(QWidget* parent)
    : QWidget(parent),
      icon_widget_(new QSvgWidget(QStringLiteral(":/icons/capsLock.svg"), this))
{
    setWindowFlags(
        Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    auto* layout = new QHBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(icon_widget_);

    icon_widget_->setFixedSize(48, 48);
    setFixedSize(48, 48);
}

void OsdWidget::showLockState(bool enabled)
{
    if (!enabled)
    {
        hide();
        return;
    }

    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());

    if (!screen)
    {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect area = screen->availableGeometry();

    constexpr int rightMargin  = 48;
    constexpr int bottomMargin = 48;

    move(area.right() - width() - rightMargin + 1,
         area.bottom() - height() - bottomMargin + 1);
    show();

    raise();
}
