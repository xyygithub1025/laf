// LAF OS Library
// Copyright (C) 2020-2021  Igara Studio S.A.
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

WinSystem::WinSystem() { }
WinSystem::~WinSystem() { }

void WinSystem::setAppName(const std::string& appName)
{
  m_appName = appName;
}

void WinSystem::setTabletAPI(TabletAPI api)
{
  m_tabletAPI = api;
}

bool WinSystem::isKeyPressed(KeyScancode scancode)
{
  return win_is_key_pressed(scancode);
}

int WinSystem::getUnicodeFromScancode(KeyScancode scancode)
{
  return win_get_unicode_from_scancode(scancode);
}

ScreenRef WinSystem::mainScreen()
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

void WinSystem::listScreens(ScreenList& list)
{
  EnumDisplayMonitors(
    nullptr, nullptr,
    list_screen_enumproc,
    (LPARAM)&list);
}

} // namespace os
