// LAF OS Library
// Copyright (C) 2021  Igara Studio S.A.
// Copyright (C) 2016  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_X11_SYSTEM_H
#define OS_X11_SYSTEM_H
#pragma once

#include "os/common/system.h"
#include "os/x11/keys.h"
#include "os/x11/screen.h"
#include "os/x11/x11.h"

namespace os {

class SystemX11 : public CommonSystem {
public:
  bool isKeyPressed(KeyScancode scancode) override {
    return x11_is_key_pressed(scancode);
  }

  int getUnicodeFromScancode(KeyScancode scancode) override {
    return x11_get_unicode_from_scancode(scancode);
  }

  ScreenRef mainScreen() override {
    return make_ref<ScreenX11>(
      XDefaultScreen(X11::instance()->display()));
  }

  void listScreens(ScreenList& list) override {
    const int nscreen = XScreenCount(X11::instance()->display());
    for (int screen=0; screen<nscreen; screen++)
      list.push_back(make_ref<ScreenX11>(screen));
  }

};

} // namespace os

#endif
