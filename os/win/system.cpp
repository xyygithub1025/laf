// LAF OS Library
// Copyright (C) 2020-2021  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/win/system.h"

#include "gfx/color.h"
#include "os/win/screen.h"

#include <limits>

namespace os {

bool win_is_key_pressed(KeyScancode scancode);
int win_get_unicode_from_scancode(KeyScancode scancode);

static const gfx::Point kUnknownPos(std::numeric_limits<int>::min(),
                                    std::numeric_limits<int>::min());

SystemWin::SystemWin()
  : m_screenMousePos(kUnknownPos)
{
}

SystemWin::~SystemWin()
{
}

void SystemWin::setAppName(const std::string& appName)
{
  m_appName = appName;
}

void SystemWin::setTabletAPI(TabletAPI api)
{
  m_tabletAPI = api;

  // If the user selects the wintab API again, we remove any possible
  // file indicating a crash in the past.
  if (m_tabletAPI == TabletAPI::Wintab ||
      m_tabletAPI == TabletAPI::WintabPackets) {
    m_wintabApi.resetCrashFileIfPresent();
  }
}

bool SystemWin::isKeyPressed(KeyScancode scancode)
{
  return win_is_key_pressed(scancode);
}

int SystemWin::getUnicodeFromScancode(KeyScancode scancode)
{
  return win_get_unicode_from_scancode(scancode);
}

gfx::Point SystemWin::mousePosition() const
{
  // We cannot use GetCursorPos() directly because it doesn't work
  // when have a pen connected to a notebook.
  if (m_screenMousePos != kUnknownPos) {
    return m_screenMousePos;
  }
  else {
    POINT pt;
    GetCursorPos(&pt);
    return gfx::Point(pt.x, pt.y);
  }
}

void SystemWin::setMousePosition(const gfx::Point& screenPosition)
{
  SetCursorPos(screenPosition.x, screenPosition.y);
}

gfx::Color SystemWin::getColorFromScreen(const gfx::Point& screenPosition) const
{
  HDC dc = GetDC(nullptr);
  COLORREF c = GetPixel(dc, screenPosition.x, screenPosition.y);
  ReleaseDC(nullptr, dc);
  return gfx::rgba(GetRValue(c),
                   GetGValue(c),
                   GetBValue(c));
}

ScreenRef SystemWin::mainScreen()
{
  // This is one of three possible ways to get the primary monitor
  // https://devblogs.microsoft.com/oldnewthing/20141106-00/?p=43683
  HMONITOR monitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
  if (monitor)
    return make_ref<ScreenWin>(monitor);
  else
    return nullptr;
}

static BOOL CALLBACK list_screen_enumproc(HMONITOR monitor,
                                          HDC hdc, LPRECT rc,
                                          LPARAM data)
{
  auto list = (ScreenList*)data;
  list->push_back(make_ref<ScreenWin>(monitor));
  return TRUE;
}

void SystemWin::listScreens(ScreenList& list)
{
  EnumDisplayMonitors(
    nullptr, nullptr,
    list_screen_enumproc,
    (LPARAM)&list);
}

void SystemWin::_clearInternalMousePosition()
{
  m_screenMousePos = kUnknownPos;
}

void SystemWin::_setInternalMousePosition(const gfx::Point& pos)
{
  m_screenMousePos = pos;
}

void SystemWin::_setInternalMousePosition(const Event& ev)
{
  ASSERT(ev.window());
  if (!ev.window()) {           // Invalid Event state
    m_screenMousePos = kUnknownPos;
    return;
  }
  m_screenMousePos = ev.window()->pointToScreen(ev.position());
}

} // namespace os
