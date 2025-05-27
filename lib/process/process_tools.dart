import 'dart:ffi';
import 'dart:io';

import 'package:ffi/ffi.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter_tools_list/process/process_tools_bindings_generated.dart';

const String _libName = 'flutter_tools_list';

/// The dynamic library in which the symbols for [FlutterToolsListBindings] can be found.
final DynamicLibrary _dylib = () {
  if (Platform.isWindows) {
    return DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final ProcessToolsListBindings _bindings = ProcessToolsListBindings(_dylib);

/// 判断端口号是否被占用
bool isPortInUse(int port) {
  if (_bindings.is_port_in_use(port) == 1) {
    return true;
  } else {
    return false;
  }
}

/// 通过进程名取进程ID，失败返回0
int getPidByName(String name) {
  final Pointer<Char> namePointer = name.toNativeUtf8().cast<Char>();
  try {
    return _bindings.get_pid_by_name(namePointer);
  } finally {
    calloc.free(namePointer);
  }
}

/// 通过端口号获取进程PID，失败返回0
int getPidByPort(int pid) => _bindings.get_pid_by_port(pid);

/// ### [异常]
/// 通过pid获取进程路径（异步版本 - 使用compute）
Future<String> getPidByPath(int pid) async {
  return compute(_getPidByPathInIsolate, pid);
}

// 在Isolate中执行的函数
String _getPidByPathInIsolate(int pid) {
  // 在Isolate中创建新的绑定
  final dylib = DynamicLibrary.open('$_libName.dll');
  final isolateBindings = ProcessToolsListBindings(dylib);

  final Pointer<Char> namePointer = isolateBindings.get_pid_by_path(pid);
  try {
    if (namePointer == nullptr || namePointer.address == 0) {
      return "";
    }
    return namePointer.cast<Utf8>().toDartString();
  } finally {
    // 释放C层分配的内存
    if (namePointer != nullptr && namePointer.address != 0) {
      malloc.free(namePointer);
    }
  }
}
