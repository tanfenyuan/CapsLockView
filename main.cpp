#include <windows.h>
#include <shellapi.h>

namespace
{
constexpr wchar_t kWindowClassName[] = L"CapsTipNativeWindow";

constexpr UINT kTrayCallbackMessage = WM_APP + 1;
constexpr UINT kExitCommand         = 1001;

constexpr int kIndicatorSize = 64;
constexpr int kRightMargin   = 24;
constexpr int kBottomMargin  = 64;

struct AppState
{
    bool capsLockEnabled = false;
    bool capsKeyDown     = false;
    NOTIFYICONDATAW trayIcon{};
};

void updateIndicatorPosition(HWND window)
{
    POINT cursorPosition{};
    GetCursorPos(&cursorPosition);

    const HMONITOR monitor = MonitorFromPoint(cursorPosition, MONITOR_DEFAULTTONEAREST);

    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfoW(monitor, &monitorInfo))
    {
        return;
    }

    const RECT& workArea = monitorInfo.rcWork;
    const int x = workArea.right - kIndicatorSize - kRightMargin;
    const int y = workArea.bottom - kIndicatorSize - kBottomMargin;

    SetWindowPos(window,
                 HWND_TOPMOST,
                 x,
                 y,
                 kIndicatorSize,
                 kIndicatorSize,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void updateIndicatorVisibility(HWND window, const AppState& state)
{
    if (!state.capsLockEnabled)
    {
        ShowWindow(window, SW_HIDE);
        return;
    }

    updateIndicatorPosition(window);
    InvalidateRect(window, nullptr, TRUE);
}

void paintIndicator(HWND window)
{
    PAINTSTRUCT paint{};
    HDC deviceContext = BeginPaint(window, &paint);

    RECT clientRect{};
    GetClientRect(window, &clientRect);

    const HBRUSH backgroundBrush = CreateSolidBrush(RGB(44, 44, 44));
    const HBRUSH iconBrush       = CreateSolidBrush(RGB(255, 255, 255));
    const HPEN transparentPen    = static_cast<HPEN>(GetStockObject(NULL_PEN));

    const HGDIOBJ previousBrush = SelectObject(deviceContext, backgroundBrush);
    const HGDIOBJ previousPen   = SelectObject(deviceContext, transparentPen);

    RoundRect(deviceContext,
              clientRect.left,
              clientRect.top,
              clientRect.right,
              clientRect.bottom,
              12,
              12);

    SelectObject(deviceContext, iconBrush);

    // Caps Lock 向上箭头。
    POINT arrow[] = {
        {32, 12},
        {52, 33},
        {43, 33},
        {43, 45},
        {21, 45},
        {21, 33},
        {12, 33},
    };

    Polygon(deviceContext, arrow, static_cast<int>(sizeof(arrow) / sizeof(arrow[0])));
    Rectangle(deviceContext, 21, 51, 44, 56);

    SelectObject(deviceContext, previousPen);
    SelectObject(deviceContext, previousBrush);

    DeleteObject(iconBrush);
    DeleteObject(backgroundBrush);

    EndPaint(window, &paint);
}

bool registerRawKeyboard(HWND window)
{
    RAWINPUTDEVICE keyboard{};
    keyboard.usUsagePage = 0x01;
    keyboard.usUsage     = 0x06;
    keyboard.dwFlags     = RIDEV_INPUTSINK;
    keyboard.hwndTarget  = window;

    return RegisterRawInputDevices(&keyboard, 1, sizeof(keyboard)) == TRUE;
}

void unregisterRawKeyboard()
{
    RAWINPUTDEVICE keyboard{};
    keyboard.usUsagePage = 0x01;
    keyboard.usUsage     = 0x06;
    keyboard.dwFlags     = RIDEV_REMOVE;
    keyboard.hwndTarget  = nullptr;

    RegisterRawInputDevices(&keyboard, 1, sizeof(keyboard));
}

void addTrayIcon(HWND window, AppState& state)
{
    state.trayIcon.cbSize           = sizeof(state.trayIcon);
    state.trayIcon.hWnd             = window;
    state.trayIcon.uID              = 1;
    state.trayIcon.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    state.trayIcon.uCallbackMessage = kTrayCallbackMessage;
    state.trayIcon.hIcon            = LoadIconW(nullptr, IDI_INFORMATION);

    wcscpy_s(state.trayIcon.szTip, L"CapsTip");
    Shell_NotifyIconW(NIM_ADD, &state.trayIcon);
}

void showTrayMenu(HWND window)
{
    POINT cursorPosition{};
    GetCursorPos(&cursorPosition);

    const HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kExitCommand, L"退出");

    SetForegroundWindow(window);
    TrackPopupMenu(menu,
                   TPM_RIGHTBUTTON | TPM_BOTTOMALIGN,
                   cursorPosition.x,
                   cursorPosition.y,
                   0,
                   window,
                   nullptr);

    DestroyMenu(menu);
    PostMessageW(window, WM_NULL, 0, 0);
}

void processRawInput(HWND window, AppState& state, LPARAM inputHandle)
{
    RAWINPUT input{};
    UINT inputSize = sizeof(input);

    const UINT bytesRead = GetRawInputData(reinterpret_cast<HRAWINPUT>(inputHandle),
                                           RID_INPUT,
                                           &input,
                                           &inputSize,
                                           sizeof(RAWINPUTHEADER));

    if (bytesRead == static_cast<UINT>(-1) || input.header.dwType != RIM_TYPEKEYBOARD)
    {
        return;
    }

    const RAWKEYBOARD& keyboard = input.data.keyboard;

    if (keyboard.VKey != VK_CAPITAL)
    {
        return;
    }

    if (keyboard.Flags & RI_KEY_BREAK)
    {
        state.capsKeyDown = false;
        return;
    }

    // 防止极少数键盘对 Caps Lock 产生自动重复事件。
    if (state.capsKeyDown)
    {
        return;
    }

    state.capsKeyDown     = true;
    state.capsLockEnabled = !state.capsLockEnabled;

    updateIndicatorVisibility(window, state);
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* state = reinterpret_cast<AppState*>(GetWindowLongPtrW(window, GWLP_USERDATA));

    if (message == WM_NCCREATE)
    {
        const auto* createInfo = reinterpret_cast<CREATESTRUCTW*>(lParam);
        state = static_cast<AppState*>(createInfo->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    }

    switch (message)
    {
    case WM_INPUT:
        if (state)
        {
            processRawInput(window, *state, lParam);
        }
        return 0;

    case WM_PAINT:
        paintIndicator(window);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_NCHITTEST:
        return HTTRANSPARENT;

    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;

    case kTrayCallbackMessage:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU)
        {
            showTrayMenu(window);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == kExitCommand)
        {
            DestroyWindow(window);
            return 0;
        }
        break;

    case WM_DESTROY:
        unregisterRawKeyboard();
        if (state)
        {
            Shell_NotifyIconW(NIM_DELETE, &state->trayIcon);
        }
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    AppState state{};
    state.capsLockEnabled = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

    WNDCLASSEXW windowClass{};
    windowClass.cbSize        = sizeof(windowClass);
    windowClass.hInstance     = instance;
    windowClass.lpfnWndProc   = windowProcedure;
    windowClass.lpszClassName = kWindowClassName;
    windowClass.hCursor       = LoadCursorW(nullptr, IDC_ARROW);

    if (!RegisterClassExW(&windowClass))
    {
        return 1;
    }

    const DWORD extendedStyle = WS_EX_TOPMOST |
                                WS_EX_TOOLWINDOW |
                                WS_EX_NOACTIVATE |
                                WS_EX_TRANSPARENT |
                                WS_EX_LAYERED;

    const HWND window = CreateWindowExW(extendedStyle,
                                        kWindowClassName,
                                        L"CapsTip",
                                        WS_POPUP,
                                        0,
                                        0,
                                        kIndicatorSize,
                                        kIndicatorSize,
                                        nullptr,
                                        nullptr,
                                        instance,
                                        &state);

    if (!window)
    {
        return 1;
    }

    SetLayeredWindowAttributes(window, 0, 235, LWA_ALPHA);

    const HRGN roundedRegion = CreateRoundRectRgn(0,
                                                   0,
                                                   kIndicatorSize + 1,
                                                   kIndicatorSize + 1,
                                                   12,
                                                   12);
    SetWindowRgn(window, roundedRegion, FALSE);

    if (!registerRawKeyboard(window))
    {
        MessageBoxW(nullptr,
                    L"无法注册 Raw Input 键盘设备。",
                    L"CapsTip",
                    MB_OK | MB_ICONERROR);
        DestroyWindow(window);
        return 1;
    }

    addTrayIcon(window, state);
    updateIndicatorVisibility(window, state);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
