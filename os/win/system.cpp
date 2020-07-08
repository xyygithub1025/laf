// LAF OS Library
// Copyright (C) 2020  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/win/system.h"

#include "os/win/screen.h"

namespace os {

bool win_is_key_pressed(KeyScancode scancode);
int win_get_unicode_from_scancode(KeyScancode scancode);

WindowSystem::WindowSystem() { }
WindowSystem::~WindowSystem() { }

void WindowSystem::setAppName(const std::string& appName)
{
  m_appName = appName;
}

void WindowSystem::setTabletAPI(TabletAPI api)
{
  m_tabletAPI = api;
}

bool WindowSystem::isKeyPressed(KeyScancode scancode)
{
  return win_is_key_pressed(scancode);
}

int WindowSystem::getUnicodeFromScancode(KeyScancode scancode)
{
  return win_get_unicode_from_scancode(scancode);
}

ScreenRef WindowSystem::mainScreen()
{
  // This is one of three possible ways to get the primary monitor
  // https://devblogs.microsoft.com/oldnewthing/20141106-00/?p=43683
  HMONITOR monitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
  if (monitor)
    return make_ref<WinScreen>(monitor);
  else
    return nullptr;
}

static BOOL CALLBACK list_screen_enumproc(HMONITOR monitor,
                                          HDC hdc, LPRECT rc,
                                          LPARAM data)
{
  auto list = (ScreenList*)data;
  list->push_back(make_ref<WinScreen>(monitor));
  return TRUE;
}

void WindowSystem::listScreens(ScreenList& list)
{
  EnumDisplayMonitors(
    nullptr, nullptr,
    list_screen_enumproc,
    (LPARAM)&list);
}

} // namespace os
