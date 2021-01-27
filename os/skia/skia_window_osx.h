// LAF OS Library
// Copyright (C) 2018-2021  Igara Studio S.A.
// Copyright (C) 2012-2016  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_SKIA_SKIA_WINDOW_OSX_INCLUDED
#define OS_SKIA_SKIA_WINDOW_OSX_INCLUDED
#pragma once

#include "base/disable_copying.h"
#include "os/color_space.h"
#include "os/native_cursor.h"
#include "os/screen.h"

#include <string>

namespace os {

class DisplaySpec;
class EventQueue;
class SkiaDisplay;
class Surface;

class SkiaWindow {
public:
  enum class Backend { NONE, GL };

  SkiaWindow(EventQueue* queue, SkiaDisplay* display,
             const DisplaySpec& spec);
  ~SkiaWindow();

  os::ScreenRef screen() const;
  os::ColorSpaceRef colorSpace() const;
  int scale() const;
  void setScale(int scale);
  bool isVisible() const;
  void setVisible(bool visible);
  void maximize();
  bool isMaximized() const;
  bool isMinimized() const;
  bool isFullscreen() const;
  void setFullscreen(bool state);
  gfx::Size clientSize() const;
  gfx::Size restoredSize() const;
  gfx::Rect frame() const;
  gfx::Rect contentRect() const;
  std::string title() const;
  void setTitle(const std::string& title);
  void captureMouse();
  void releaseMouse();
  void setMousePosition(const gfx::Point& position);
  bool setNativeMouseCursor(NativeCursor cursor);
  bool setNativeMouseCursor(const Surface* surface,
                            const gfx::Point& focus,
                            const int scale);
  void invalidateRegion(const gfx::Region& rgn);
  std::string getLayout() { return ""; }
  void setLayout(const std::string& layout) { }
  void setTranslateDeadKeys(bool state);
  void* handle();

private:
  void destroyImpl();

  class Impl;
  Impl* __strong m_impl;

  DISABLE_COPYING(SkiaWindow);
};

} // namespace os

#endif
