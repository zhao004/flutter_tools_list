# Flutter Tools List 插件实现的功能

目前该插件已实现以下方法：

## 基本工具函数

- `sum(int a, int b)`: 计算两个整数的和（短时间运行的示例函数）
- `sumAsync(int a, int b)`: 在独立的isolate中计算两个整数的和（长时间运行的示例函数）

## 进程相关工具函数

- `getPidByName(String name)`: 通过进程名称获取进程ID，失败返回0
- `getPidByPort(int port)`: 通过端口号获取占用该端口的进程ID，失败返回0
- `isPortInUse(int port)`: 判断指定端口号是否被占用，返回1表示被占用，0表示未占用
- `getPathByPid(int pid)`: 通过进程ID获取进程的完整路径，成功返回路径字符串，失败返回空字符串

目前插件已在Windows平台实现。