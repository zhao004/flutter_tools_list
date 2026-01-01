# flutter_tools_list_example

演示如何使用 flutter_tools_list 插件的示例应用。

## 功能演示

本示例应用演示了以下功能：

1. **检查端口占用**: 使用 `isPortInUse()` 检查指定端口是否被占用
2. **通过进程名获取PID**: 使用 `getPidByName()` 查找进程ID
3. **通过端口获取PID**: 使用 `getPidByPort()` 查找占用端口的进程
4. **获取进程路径**: 使用 `getPidByPath()` 获取进程的完整文件路径

## 运行示例

### Windows 平台

```bash
cd example
flutter run -d windows
```

或者构建 Release 版本：

```bash
flutter build windows --release
```

## 示例代码

查看 `lib/main.dart` 了解如何在您的应用中使用这些功能。

## 使用说明

```dart
import 'package:flutter_tools_list/process/process_tools.dart';

// 检查端口
bool inUse = isPortInUse(8080);

// 获取 Chrome 进程的 PID
int pid = getPidByName('chrome.exe');

// 获取占用 8080 端口的进程 PID
int portPid = getPidByPort(8080);

// 获取进程的完整路径（异步）
String path = await getPidByPath(pid);
```

## 更多资源

- [Flutter 官方文档](https://docs.flutter.dev/)
- [Dart FFI 文档](https://dart.dev/guides/libraries/c-interop)
- [插件开发指南](https://docs.flutter.dev/development/packages-and-plugins/developing-packages)

