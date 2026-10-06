#pragma once

/**
* @file    RawKeyboardMonitor.h
* @author  tanfenyuan@outlook.com
* @date    2026-10-06
*/


#include <QObject>
#include <QAbstractNativeEventFilter>

#include <Windows.h>


class RawKeyboardMonitor :
    public QObject,
    public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit RawKeyboardMonitor(QObject* parent = nullptr);
    ~RawKeyboardMonitor() override;

    bool start(HWND hwnd);

    void stop();

    [[nodiscard]] bool capsLockEnabled() const noexcept
    {
        return caps_lock_;
    }

signals:
    void capsLockChanged(bool enabled);

protected:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    void updateLockState(UINT virtualKey);
    HWND hwnd_ = nullptr;

    bool started_{false};
    bool caps_lock_{false};
};
