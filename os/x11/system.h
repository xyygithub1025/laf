// LAF OS Library
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

class X11System : public CommonSystem {
public:
  bool isKeyPressed(KeyScancode scancode) override {
    return x11_is_key_pressed(scancode);
  }

  int getUnicodeFromScancode(KeyScancode scancode) override {
    return x11_get_unicode_from_scancode(scancode);
  }

  ScreenRef mainScreen() override {
    return make_ref<X11Screen>(
      XDefaultScreen(X11::instance()->display()));
  }

  void listScreens(ScreenList& list) override {
    const int nscreen = XScreenCount(X11::instance()->display());
    for (int screen=0; screen<nscreen; screen++)
      list.push_back(make_ref<X11Screen>(screen));
  }

};

} // namespace os

#endif
