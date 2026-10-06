#include <QApplication>
#include <QDebug>
#include "RawKeyboardMonitor.h"
#include "OsdWidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QWidget messageWindows;
    messageWindows.setAttribute(Qt::WA_NativeWindow);
    HWND hwnd = reinterpret_cast<HWND>(messageWindows.winId());


    RawKeyboardMonitor monitor;
    OsdWidget          osd;

    if (!monitor.start(hwnd))
    {
        qCritical() << "RegisterInput failed" << GetLastError();
        return 1;
    }

    QObject::connect(&monitor,
                     &RawKeyboardMonitor::capsLockChanged,
                     &osd,
                     [&osd](bool enabled)
                     {
                         osd.showLockState(enabled);
                     });

    // 如果程序启动前 Caps Lock 已经开启，也应立即显示图标。
    osd.showLockState(monitor.capsLockEnabled());

    return app.exec();
}
