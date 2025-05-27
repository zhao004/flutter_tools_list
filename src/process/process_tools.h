#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define FFI_PLUGIN_EXPORT __declspec(dllexport)

// Get process PID by name, returns 0 if failed
FFI_PLUGIN_EXPORT int get_pid_by_name(const char *process_name);
// Get process PID by port number, returns 0 if failed
FFI_PLUGIN_EXPORT int get_pid_by_port(int port);
// Check if port is in use, returns 1 if in use, 0 if not
FFI_PLUGIN_EXPORT int is_port_in_use(int port);
// Get process path by PID, returns path if successful, empty string if failed
FFI_PLUGIN_EXPORT const char *get_pid_by_path(int pid);
