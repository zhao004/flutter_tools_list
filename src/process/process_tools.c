#define _CRT_SECURE_NO_WARNINGS
#include "process_tools.h"

#include <tlhelp32.h>
#include <psapi.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

// Get process PID by name, returns 0 if failed
FFI_PLUGIN_EXPORT int get_pid_by_name(const char *process_name)
{
  // Input validation
  if (process_name == NULL || process_name[0] == '\0')
  {
    return 0;
  }

  HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE)
  {
    return 0;
  }

  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (!Process32First(hProcessSnap, &pe32))
  {
    CloseHandle(hProcessSnap);
    return 0;
  }

  // Allocate buffer outside loop for better performance
  char exeName[MAX_PATH];
  int pid = 0;

  do
  {
    // Convert wide character to multibyte character for comparison
    if (WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, exeName, MAX_PATH, NULL, NULL) > 0)
    {
      // Use case-insensitive comparison (more robust on Windows)
      if (_stricmp(exeName, process_name) == 0)
      {
        pid = pe32.th32ProcessID;
        break;
      }
    }
  } while (Process32Next(hProcessSnap, &pe32));

  CloseHandle(hProcessSnap);
  return pid;
}

// Helper function to check TCP table for port
// check_listen_only: if 1, only check LISTEN state (for port availability)
//                    if 0, check all states (for finding PID by port)
static int check_tcp_port(int port, DWORD *out_pid, int check_listen_only)
{
  DWORD dwSize = 0;
  DWORD dwRetVal = 0;

  // Get required buffer size
  MIB_TCPTABLE_OWNER_PID *pTcpTable = NULL;
  dwRetVal = GetExtendedTcpTable(NULL, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);

  if (dwRetVal != ERROR_INSUFFICIENT_BUFFER)
  {
    return 0;
  }

  pTcpTable = (MIB_TCPTABLE_OWNER_PID *)malloc(dwSize);
  if (pTcpTable == NULL)
  {
    return 0;
  }

  // Get TCP table
  dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);

  if (dwRetVal != NO_ERROR)
  {
    free(pTcpTable);
    return 0;
  }

  // Search for port
  for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++)
  {
    int local_port = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
    if (local_port == port)
    {
      // If checking for port availability, only consider LISTEN state
      // This prevents false positives from TIME_WAIT, CLOSE_WAIT, etc.
      if (check_listen_only)
      {
        if (pTcpTable->table[i].dwState != MIB_TCP_STATE_LISTEN)
        {
          continue; // Skip non-LISTEN states
        }
      }

      if (out_pid)
      {
        *out_pid = pTcpTable->table[i].dwOwningPid;
      }
      free(pTcpTable);
      return 1;
    }
  }

  free(pTcpTable);
  return 0;
}

// Helper function to check UDP table for port
static int check_udp_port(int port, DWORD *out_pid)
{
  DWORD dwSize = 0;
  DWORD dwRetVal = 0;

  // Get required buffer size
  MIB_UDPTABLE_OWNER_PID *pUdpTable = NULL;
  dwRetVal = GetExtendedUdpTable(NULL, &dwSize, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);

  if (dwRetVal != ERROR_INSUFFICIENT_BUFFER)
  {
    return 0;
  }

  pUdpTable = (MIB_UDPTABLE_OWNER_PID *)malloc(dwSize);
  if (pUdpTable == NULL)
  {
    return 0;
  }

  // Get UDP table
  dwRetVal = GetExtendedUdpTable(pUdpTable, &dwSize, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);

  if (dwRetVal != NO_ERROR)
  {
    free(pUdpTable);
    return 0;
  }

  // Search for port
  for (DWORD i = 0; i < pUdpTable->dwNumEntries; i++)
  {
    int local_port = ntohs((u_short)pUdpTable->table[i].dwLocalPort);
    if (local_port == port)
    {
      if (out_pid)
      {
        *out_pid = pUdpTable->table[i].dwOwningPid;
      }
      free(pUdpTable);
      return 1;
    }
  }

  free(pUdpTable);
  return 0;
}

// Get process PID by port number, returns 0 if failed
// Much faster than using netstat command - uses Windows API directly
FFI_PLUGIN_EXPORT int get_pid_by_port(int port)
{
  // Input validation
  if (port <= 0 || port > 65535)
  {
    return 0;
  }

  DWORD pid = 0;

  // Check TCP ports first (more common)
  // Use 0 for check_listen_only to check all TCP states
  if (check_tcp_port(port, &pid, 0))
  {
    return (int)pid;
  }

  // Check UDP ports
  if (check_udp_port(port, &pid))
  {
    return (int)pid;
  }

  return 0;
}

// Check if port is in use, returns 1 if in use, 0 if not
// Optimized to avoid unnecessary PID retrieval
// Only checks LISTEN state to avoid false positives from TIME_WAIT, CLOSE_WAIT, etc.
FFI_PLUGIN_EXPORT int is_port_in_use(int port)
{
  // Input validation
  if (port <= 0 || port > 65535)
  {
    return 0;
  }

  // Check TCP and UDP without retrieving PID (faster)
  // Use 1 for check_listen_only to only check LISTEN state
  return check_tcp_port(port, NULL, 1) || check_udp_port(port, NULL);
}

// Get process path by PID, returns path if successful, empty string if failed
// Optimized to use QueryFullProcessImageNameA as primary method (Vista+)
// Falls back to GetModuleFileNameEx for older systems
FFI_PLUGIN_EXPORT const char *get_pid_by_path(int pid)
{
  static char path_buffer[MAX_PATH];
  path_buffer[0] = '\0'; // Initialize to empty string

  // Input validation - prevent invalid PID
  if (pid <= 0 || pid == 4) // PID 0 and 4 are system processes
  {
    return path_buffer;
  }

  // Try QueryFullProcessImageNameA first (Vista+, faster, more reliable)
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (hProcess != NULL)
  {
    DWORD pathSize = MAX_PATH;
    if (QueryFullProcessImageNameA(hProcess, 0, path_buffer, &pathSize) != 0)
    {
      CloseHandle(hProcess);
      return path_buffer; // Success
    }
    CloseHandle(hProcess);
  }

  // Fallback: Try with more permissions for GetModuleFileNameEx
  // This is statically linked via psapi.h, no need for LoadLibrary
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (hProcess != NULL)
  {
    DWORD result = GetModuleFileNameExA(hProcess, NULL, path_buffer, MAX_PATH);
    CloseHandle(hProcess);

    if (result == 0)
    {
      path_buffer[0] = '\0'; // Clear buffer on failure
    }
  }

  return path_buffer;
}