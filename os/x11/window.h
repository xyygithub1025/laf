// LAF OS Library
// Copyright (C) 2018-2021  Igara Studio S.A.
// Copyright (C) 2016-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_X11_WINDOW_INCLUDED
#define OS_X11_WINDOW_INCLUDED
#pragma once

#include "base/time.h"
#include "gfx/border.h"
#include "gfx/color_space.h"    // Include here avoid error with None
#include "gfx/fwd.h"
#include "gfx/size.h"
#include "os/color_space.h"
#include "os/event.h"
#include "os/native_cursor.h"
#include "os/screen.h"
#include "os/surface_list.h"
#include "os/window.h"

#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <cstring>
#include <string>

namespace os {

class Surface;
class WindowSpec;

class WindowX11 : public Window {
public:
  WindowX11(::Display* display,
            const WindowSpec& spec);
  ~WindowX11();

  void queueEvent(Event& ev);

  os::ScreenRef screen() const override;
  os::ColorSpaceRef colorSpace() const override;

  int scale() const override { return m_scale; }
  void setScale(const int scale) override;

  bool isVisible() const override;
  void setVisible(bool visible) override;

  void activate() override;
  void maximize() override;
  void minimize() override;
  bool isMaximized() const override;
  bool isMinimized() const override;

  bool isFullscreen() const;
  void setFullscreen(bool state);

  void setTitle(const std::string& title);
  void setIcons(const SurfaceList& icons);

  gfx::Rect frame() const;
  gfx::Rect contentRect() const;
  std::string title() const;

  gfx::Size clientSize() const;
  gfx::Size restoredSize() const;
  void captureMouse();
  void releaseMouse();
  void setMousePosition(const gfx::Point& position);
  void invalidateRegion(const gfx::Region& rgn);
  bool setNativeMouseCursor(NativeCursor cursor);
  bool setNativeMouseCursor(const os::Surface* surface,
                            const gfx::Point& focus,
                            const int scale);

  void performWindowAction(const WindowAction action,
                           const Event* event) override;

  ::Display* x11display() const { return m_display; }
  ::Window x11window() const { return m_window; }
  ::GC gc() const { return m_gc; }

  NativeHandle nativeHandle() const override { return (NativeHandle)x11window(); }

  void setTranslateDeadKeys(bool state) {
    // TODO
  }

  void processX11Event(XEvent& event);
  static WindowX11* getPointerFromHandle(::Window handle);

protected:
  virtual void onQueueEvent(Event& event) = 0;
  virtual void onPaint(const gfx::Rect& rc) = 0;
  virtual void onResize(const gfx::Size& sz) = 0;

private:
  void setWMClass(const std::string& res_class);
  void setAllowedActions();
  bool setX11Cursor(::Cursor xcursor);
  static void addWindow(WindowX11* window);
  static void removeWindow(WindowX11* window);

  ::Display* m_display;
  ::Window m_window;
  ::GC m_gc;
  ::Cursor m_cursor;
  ::XcursorImage* m_xcursorImage;
  ::XIC m_xic;
  int m_scale;
  gfx::Point m_lastMousePos;
  gfx::Size m_lastClientSize;
  gfx::Border m_frameExtents;
  bool m_initializingFromFrame = false;
  bool m_initializingActions = true;
  bool m_fullscreen = false;
  bool m_borderless = false;
  bool m_closable = false;
  bool m_maximizable = false;
  bool m_minimizable = false;
  bool m_resizable = false;

  // Double-click info
  Event::MouseButton m_doubleClickButton;
  base::tick_t m_doubleClickTick;
};

} // namespace os

#endif
