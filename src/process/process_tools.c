#define _CRT_SECURE_NO_WARNINGS
#include "process_tools.h"

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h> // Must include this header to use GetModuleFileNameEx

// Get process PID by name, returns 0 if failed
FFI_PLUGIN_EXPORT int get_pid_by_name(const char *process_name)
{
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;
  int pid = 0;
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE)
  {
    return 0;
  }
  pe32.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(hProcessSnap, &pe32))
  {
    CloseHandle(hProcessSnap);
    return 0;
  }
  do
  { // Convert wide character to multibyte character for comparison
    char exeName[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, exeName, MAX_PATH, NULL, NULL);

    if (strcmp(exeName, process_name) == 0)
    {
      pid = pe32.th32ProcessID;
      break;
    }
  } while (Process32Next(hProcessSnap, &pe32));
  CloseHandle(hProcessSnap);
  return pid;
}

// Get process PID by port number, returns 0 if failed
FFI_PLUGIN_EXPORT int get_pid_by_port(int port)
{
  // Get process information using netstat command on Windows
  FILE *fp = _popen("netstat -ano", "r");
  if (!fp)
    return 0;
  char line[512];
  int pid = 0;
  char local_addr[64];
  int local_port;
  while (fgets(line, sizeof(line), fp))
  {
    if (strstr(line, "TCP") || strstr(line, "UDP"))
    {
      if (sscanf_s(line, "%*s %63[^:]:%d", local_addr, (unsigned)sizeof(local_addr), &local_port) == 2)
      {
        if (local_port == port)
        { // Extract PID more robustly
          int tmp_pid = 0;
          char *pid_str = strrchr(line, ' ');
          if (pid_str)
          {
            while (*pid_str == ' ')
              pid_str++; // Skip extra spaces
            tmp_pid = atoi(pid_str);
            pid = tmp_pid;
            break;
          }
        }
      }
    }
  }
  _pclose(fp);
  return pid;
}

// Check if port is in use, returns 1 if in use, 0 if not
FFI_PLUGIN_EXPORT int is_port_in_use(int port)
{
  return get_pid_by_port(port) != 0 ? 1 : 0;
}

// Get process path by PID, returns path if successful, empty string if failed
FFI_PLUGIN_EXPORT const char *get_pid_by_path(int pid)
{
  static char path_buffer[MAX_PATH];
  path_buffer[0] = '\0'; // Initialize to empty string

  // Prevent invalid PID
  if (pid <= 4)
  { // System process PIDs are usually 0-4
    return path_buffer;
  }

  // Try to get the path using the standard method - faster and does not require extra libraries
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (hProcess != NULL)
  {
    DWORD pathSize = MAX_PATH;
    if (QueryFullProcessImageNameA(hProcess, 0, path_buffer, &pathSize) != 0)
    {
      CloseHandle(hProcess);
      return path_buffer; // Successfully got the path, return immediately
    }
    CloseHandle(hProcess);
  }

  // If the above method fails, try using psapi
  HMODULE psapiDll = LoadLibraryA("psapi.dll");
  if (psapiDll == NULL)
  {
    return path_buffer; // Failed to load required library
  }

  typedef DWORD(WINAPI * GetModuleFileNameExFunc)(HANDLE, HMODULE, LPSTR, DWORD);
  GetModuleFileNameExFunc pGetModuleFileNameEx =
      (GetModuleFileNameExFunc)GetProcAddress(psapiDll, "GetModuleFileNameExA");

  if (pGetModuleFileNameEx == NULL)
  {
    FreeLibrary(psapiDll);
    return path_buffer; // Failed to get function address
  }

  // Open the process with safer flags
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (hProcess != NULL)
  {
    // Set timeout
    DWORD result = (*pGetModuleFileNameEx)(hProcess, NULL, path_buffer, MAX_PATH);
    CloseHandle(hProcess);

    // If the result is empty, clear the buffer
    if (result == 0)
    {
      path_buffer[0] = '\0';
    }
  }

  FreeLibrary(psapiDll);
  return path_buffer;
}