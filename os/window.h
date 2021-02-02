// LAF OS Library
// Copyright (c) 2018-2021  Igara Studio S.A.
// Copyright (c) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_WINDOW_H_INCLUDED
#define OS_WINDOW_H_INCLUDED
#pragma once

#include "gfx/point.h"
#include "os/color_space.h"
#include "os/native_cursor.h"
#include "os/ref.h"
#include "os/screen.h"
#include "os/surface_list.h"

#include <functional>
#include <string>

namespace os {

  class Surface;
  class Window;
  using WindowRef = Ref<Window>;

  // A window to show graphics.
  class Window : public RefCount {
  public:
    typedef void* NativeHandle;

    virtual ~Window() { }

    // Function called to handle a "live resize"/resizing loop. If
    // this is nullptr, an Event::ResizeWindow is generated when the
    // resizing is finished.
    //
    // TODO I think we should have a WindowDelegate for this instead
    //      of a public property.
    std::function<void(os::Window*)> handleResize = nullptr;

    // Real rectangle of this window (including title bar, etc.) in
    // the screen. (The scale is not involved.)
    virtual gfx::Rect frame() const = 0;

    // Rectangle of the content, the origin is 0,0 and the
    // width/height dimensions are specified in real screen pixels.
    // (The scale is not involved.)
    virtual gfx::Rect contentRect() const = 0;

    // Returns the real and current window's size (without scale applied).
    virtual int width() const = 0;
    virtual int height() const = 0;
    gfx::Rect bounds() const;

    // Returns the window when it was not maximized.
    virtual int originalWidth() const = 0;
    virtual int originalHeight() const = 0;

    // Returns the current window scale. Each pixel in the internal
    // window surface, is represented by SCALExSCALE pixels on the
    // screen.
    virtual int scale() const = 0;

    // Changes the scale.
    // The available surface size will be (Window::width() / scale,
    //                                     Window::height() / scale)
    virtual void setScale(int scale) = 0;

    // Returns true if the window is visible in the screen.
    virtual bool isVisible() const = 0;

    // Shows or hides the window (doesn't destroy it).
    virtual void setVisible(bool visible) = 0;

    // Returns the main surface to draw into this window.
    virtual Surface* surface() = 0;

    // Invalidates part of the window to be redraw in the future by
    // the OS painting messages. The region must be in non-scaled
    // coordinates.
    virtual void invalidateRegion(const gfx::Region& rgn) = 0;
    void invalidate();

    virtual void maximize() = 0;
    virtual bool isMaximized() const = 0;
    virtual bool isMinimized() const = 0;

    virtual bool isFullscreen() const = 0;
    virtual void setFullscreen(bool state) = 0;

    virtual std::string title() const = 0;
    virtual void setTitle(const std::string& title) = 0;

    virtual void setIcons(const SurfaceList& icons) { };

    virtual NativeCursor nativeMouseCursor() = 0;
    virtual bool setNativeMouseCursor(NativeCursor cursor) = 0;
    virtual bool setNativeMouseCursor(const os::Surface* cursor,
                                      const gfx::Point& focus,
                                      const int scale) = 0;

    // TODO position? relative to upper-left corner
    virtual void setMousePosition(const gfx::Point& position) = 0;

    virtual void captureMouse() = 0;
    virtual void releaseMouse() = 0;

    // Set/get the specific information to restore the exact same
    // window position (e.g. in the same monitor).
    virtual std::string getLayout() = 0;
    virtual void setLayout(const std::string& layout) = 0;

    // For Windows 8/10 only in tablet devices: Set to true if you
    // want to interpret one finger as the mouse movement and two
    // fingers as pan/scroll (true by default). If you want to pan
    // with one finger, call this function with false.
    virtual void setInterpretOneFingerGestureAsMouseMovement(bool state) { }

    // Returns the screen where this window belongs.
    virtual os::ScreenRef screen() const = 0;

    // Returns the color space of the window where the window is located.
    virtual os::ColorSpaceRef colorSpace() const = 0;

    // Returns the HWND on Windows, X11 Window, or bridged NSWindow pointer.
    virtual NativeHandle nativeHandle() const = 0;
  };

} // namespace os

#endif
