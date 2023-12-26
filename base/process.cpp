// LAF Base Library
// Copyright (c) 2021-2024 Igara Studio S.A.
// Copyright (c) 2015-2016 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/process.h"

#include "base/string.h"

#if LAF_WINDOWS
  #include <windows.h>
  #include <tlhelp32.h>
#else
  #include <signal.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif

#if LAF_MACOS
  #include <libproc.h>
#endif

namespace base {

#if LAF_WINDOWS

pid get_current_process_id()
{
  return (pid)GetCurrentProcessId();
}

bool is_process_running(pid pid)
{
  bool running = false;

  HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
  if (handle) {
    DWORD exitCode = 0;
    if (GetExitCodeProcess(handle, &exitCode)) {
      running = (exitCode == STILL_ACTIVE);
    }
    CloseHandle(handle);
  }

  return running;
}

std::string get_process_name(pid pid)
{
  bool running = false;
  HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
  if (handle) {
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(handle, &pe)) {
      do {
        if (pe.th32ProcessID == pid)
          return base::to_utf8(pe.szExeFile);
      } while (Process32Next(handle, &pe));
    }
    CloseHandle(handle);
  }
  return std::string();
}

#else

pid get_current_process_id()
{
  return (pid)getpid();
}

bool is_process_running(pid pid)
{
  return (kill(pid, 0) == 0);
}

#if LAF_MACOS

std::string get_process_name(pid pid)
{
  struct proc_bsdinfo process;
  proc_pidinfo(pid, PROC_PIDTBSDINFO, 0,
               &process, PROC_PIDTBSDINFO_SIZE);
  return process.pbi_name;
}

#else

std::string get_process_name(pid pid)
{
  // TODO implement for Linux
  return std::string();
}

#endif

#endif  // LAF_WINDOWS

} // namespace base
