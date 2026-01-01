# Flutter Tools List - Native Implementation

本目录包含插件的原生 C 代码实现。

## 实现的功能

### 进程相关工具函数 (process/)

所有函数都导出为 FFI 插件接口 (`FFI_PLUGIN_EXPORT`)：

- `get_pid_by_name(const char *process_name)`: 通过进程名称获取进程ID，失败返回0
- `get_pid_by_port(int port)`: 通过端口号获取占用该端口的进程ID，失败返回0
- `is_port_in_use(int port)`: 判断端口是否被占用，返回1表示被占用，0表示未占用
- `get_pid_by_path(int pid)`: 通过进程ID获取进程路径，失败返回空字符串

## 技术实现

### Windows 平台 (process_tools.c)

使用以下 Windows API：
- **Process Enumeration**: `CreateToolhelp32Snapshot`, `Process32First/Next` - 进程枚举
- **IP Helper API**: `GetExtendedTcpTable`, `GetExtendedUdpTable` - TCP/UDP 端口查询
- **Process Query**: `OpenProcess`, `QueryFullProcessImageNameA`, `GetModuleFileNameExA` - 进程信息查询

### 头文件包含顺序

**重要**: 必须按以下顺序包含 Windows 头文件以避免宏重定义警告：
```c
#include <winsock2.h>  // 必须在 windows.h 之前
#include <windows.h>
```

这是因为 `windows.h` 内部包含旧版的 `winsock.h`，与 `winsock2.h` 存在宏定义冲突。

## 构建配置

CMake 配置位于 `CMakeLists.txt`，链接以下库：
- `iphlpapi.lib` - IP Helper API
- `ws2_32.lib` - Windows Sockets 2

## 性能优化

1. **直接 API 调用**: 不使用命令行工具 (如 netstat)，直接调用 Windows API
2. **单次遍历**: 查询 TCP/UDP 表时只遍历一次
3. **提前退出**: 找到匹配项后立即返回
4. **内存管理**: 正确的内存分配和释放，避免泄漏
