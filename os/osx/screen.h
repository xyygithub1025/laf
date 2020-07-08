// LAF OS Library
// Copyright (c) 2020  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_OSX_SCREEN_H_INCLUDED
#define OS_OSX_SCREEN_H_INCLUDED
#pragma once

#include "os/osx/color_space.h"
#include "os/screen.h"

#include <Cocoa/Cocoa.h>

namespace os {

class OSXScreen : public Screen {
public:
  OSXScreen(NSScreen* screen) : m_screen(screen) {
    auto rc = screen.frame;
    auto wa = screen.visibleFrame;

    m_bounds.x = rc.origin.x;
    m_bounds.y = rc.origin.y;
    m_bounds.w = rc.size.width;
    m_bounds.h = rc.size.height;

    m_workarea.x = wa.origin.x;
    m_workarea.y = wa.origin.y;
    m_workarea.w = wa.size.width;
    m_workarea.h = wa.size.height;
  }
  bool isMainScreen() const override {
    return m_screen == [NSScreen mainScreen];
  }
  gfx::Rect bounds() const override { return  m_bounds; }
  gfx::Rect workarea() const override { return m_workarea; }
  os::ColorSpaceRef colorSpace() const override {
    return convert_nscolorspace_to_os_colorspace([m_screen colorSpace]);
  }
  void* nativeHandle() const override {
    return (__bridge void*)m_screen;
  }
private:
  NSScreen* m_screen;
  gfx::Rect m_bounds;
  gfx::Rect m_workarea;
};

} // namespace os

#endif
