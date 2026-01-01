# Flutter Tools List 插件实现的功能

目前该插件已实现以下方法：

## 进程相关工具函数

- `getPidByName(String name)`: 通过进程名称获取进程ID，失败返回0
- `getPidByPort(int port)`: 通过端口号获取占用该端口的进程ID，失败返回0
- `isPortInUse(int port)`: 判断指定端口号是否被占用，返回bool值
- `getPidByPath(int pid)`: 通过进程ID获取进程的完整路径（异步方法），失败返回空字符串

## 平台支持

目前插件已在 **Windows** 平台实现并测试通过。

## 技术特性

- 使用原生 Windows API (Winsock2, IP Helper API, Process API) 实现，性能优异
- 支持 TCP 和 UDP 端口查询
- 异步方法使用 Isolate 避免阻塞 UI 线程
- 完整的错误处理和输入验证

## 使用示例

```dart
import 'package:flutter_tools_list/process/process_tools.dart';

// 检查端口是否被占用
bool inUse = isPortInUse(8080);
print('Port 8080 is in use: $inUse');

// 通过进程名获取PID
int pid = getPidByName('chrome.exe');
print('Chrome PID: $pid');

// 通过端口获取占用进程的PID
int portPid = getPidByPort(8080);
print('Process using port 8080: $portPid');

// 通过PID获取进程路径（异步）
String path = await getPidByPath(pid);
print('Process path: $path');
```
