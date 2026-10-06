# CapsTip 

纯 Win32 版本，不依赖 Qt 或其他第三方运行库。

## 构建

```powershell
D:\CMake\bin\cmake.exe `
    -S D:\WORKSPACE\CapsTip\win32-demo `
    -B D:\WORKSPACE\CapsTip\build\win32-demo `
    -G "Visual Studio 18 2026" `
    -A x64

D:\CMake\bin\cmake.exe `
    --build D:\WORKSPACE\CapsTip\build\win32-demo `
    --config Release `
    --parallel
```

生成文件：

```text
D:\WORKSPACE\CapsTip\build\win32-demo\Release\caps_tip_native.exe
```

## 行为

- Caps Lock 开启：在鼠标所在显示器右下角持续显示提示。
- Caps Lock 关闭：立即隐藏提示。
- 右键单击托盘图标可以退出程序。
