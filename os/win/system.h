// LAF OS Library
// Copyright (C) 2020  Igara Studio S.A.
// Copyright (C) 2012-2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_WIN_SYSTEM_H
#define OS_WIN_SYSTEM_H
#pragma once

#include "os/common/system.h"
#include "os/win/pen.h"
#include "os/win/winapi.h"

namespace os {

bool win_is_key_pressed(KeyScancode scancode);
int win_get_unicode_from_scancode(KeyScancode scancode);

class WindowSystem : public CommonSystem {
public:
  WindowSystem() { }
  ~WindowSystem() { }

  WinAPI& winApi() { return m_winApi; }
  PenAPI& penApi() { return m_penApi; }

  void setAppName(const std::string& appName) override { m_appName = appName; }
  std::string appName() const { return m_appName; }

  void useWintabAPI(bool state) override { m_useWintabAPI = state; }
  bool useWintabAPI() const { return m_useWintabAPI; }

  bool isKeyPressed(KeyScancode scancode) override {
    return win_is_key_pressed(scancode);
  }

  int getUnicodeFromScancode(KeyScancode scancode) override {
    return win_get_unicode_from_scancode(scancode);
  }

private:
  std::string m_appName;
  bool m_useWintabAPI = true;
  WinAPI m_winApi;
  PenAPI m_penApi;
};

} // namespace os

#endif
