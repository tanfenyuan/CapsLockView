#include "RawKeyboardMonitor.h"
#include <QCoreApplication>
#include <QMetaObject>

RawKeyboardMonitor::RawKeyboardMonitor(QObject* parent)
    : QObject(parent)
{
    caps_lock_ = (GetKeyState(VK_CAPITAL) & 0X0001) != 0;
}

RawKeyboardMonitor::~RawKeyboardMonitor()
{
    stop();
}

bool RawKeyboardMonitor::start(HWND hwnd)
{
    if (started_)
    {
        return true;
    }
    if (!hwnd)
    {
        return false;
    }

    hwnd_ = hwnd;

    RAWINPUTDEVICE device{};
    device.usUsagePage = 0x01;

    device.usUsage = 0x06;

    device.dwFlags    = RIDEV_INPUTSINK;
    device.hwndTarget = hwnd;

    if (!RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE)))
    {
        return false;
    }

    QCoreApplication::instance()->installNativeEventFilter(this);
    caps_lock_ = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    started_   = true;
    return true;
}

void RawKeyboardMonitor::stop()
{
    if (!started_)
    {
        return;
    }
    QCoreApplication::instance()->removeNativeEventFilter(this);

    RAWINPUTDEVICE device{};
    device.usUsagePage = 0x01;
    device.usUsage     = 0x06;

    device.dwFlags    = RIDEV_REMOVE;
    device.hwndTarget = nullptr;

    RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE));
    hwnd_    = nullptr;
    started_ = false;
}

bool RawKeyboardMonitor::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);


    if (!started_)
    {
        return false;
    }
    MSG* msg = static_cast<MSG*>(message);

    if (!msg)
    {
        return false;
    }
    if (msg->message != WM_INPUT)
    {
        return false;
    }
    RAWINPUT raw{};
    UINT     size = sizeof(raw);

    const UINT resultSize = GetRawInputData(reinterpret_cast<HRAWINPUT>(msg->lParam),
                                            RID_INPUT,
                                            &raw,
                                            &size,
                                            sizeof(RAWINPUTHEADER));

    if (resultSize == static_cast<UINT>(-1))
    {
        return false;
    }

    if (raw.header.dwType != RIM_TYPEKEYBOARD)
    {
        return false;
    }
    const RAWKEYBOARD& keyboard = raw.data.keyboard;

    // 只处理 Caps Lock 的按下事件；普通按键和释放事件不进入 Qt 事件队列。
    if ((keyboard.Flags & RI_KEY_BREAK) || keyboard.VKey != VK_CAPITAL)
    {
        return false;
    }

    QMetaObject::invokeMethod(this,
                              [this]()
                              {
                                  updateLockState(VK_CAPITAL);
                              },
                              Qt::QueuedConnection);

    return false;
}

void RawKeyboardMonitor::updateLockState(UINT virtualKey)
{
    const bool enabled = (GetKeyState(virtualKey) & 0x0001) != 0;
    if (virtualKey == VK_CAPITAL)
    {
        if (enabled != caps_lock_)
        {
            caps_lock_ = enabled;
            emit capsLockChanged(enabled);
        }
    }
}
