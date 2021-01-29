// LAF OS Library
// Copyright (C) 2018-2021  Igara Studio S.A.
// Copyright (C) 2012-2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_OSX_WINDOW_H_INCLUDED
#define OS_OSX_WINDOW_H_INCLUDED
#pragma once

#include <Cocoa/Cocoa.h>

#include "gfx/point.h"
#include "gfx/rect.h"
#include "gfx/size.h"
#include "os/display_spec.h"
#include "os/keys.h"
#include "os/native_cursor.h"
#include "os/osx/color_space.h"
#include "os/osx/screen.h"
#include "os/osx/view.h"
#include "os/system.h"

namespace os {
  class Event;
  class Surface;
}

class OSXWindowImpl;

@class OSXWindowDelegate;

@interface OSXWindow : NSWindow {
@private
  OSXWindowImpl* __weak m_impl;
  OSXWindowDelegate* __strong m_delegate;
  OSXView* __strong m_view;
  int m_scale;
}
- (OSXWindow*)initWithImpl:(OSXWindowImpl*)impl
                      spec:(const os::DisplaySpec*)spec;
- (OSXWindowImpl*)impl;
- (void)removeImpl;
- (int)scale;
- (void)setScale:(int)scale;
- (gfx::Size)clientSize;
- (gfx::Size)restoredSize;
- (void)setMousePosition:(const gfx::Point&)position;
- (BOOL)setNativeMouseCursor:(os::NativeCursor)cursor;
- (BOOL)setNativeMouseCursor:(const os::Surface*)surface
                       focus:(const gfx::Point&)focus
                       scale:(const int)scale;
- (BOOL)canBecomeKeyWindow;
@end

class OSXWindowImpl {
public:
  void createWindow(const os::DisplaySpec& spec) {
    m_window = [[OSXWindow alloc] initWithImpl:this
                                          spec:&spec];
    m_window.releasedWhenClosed = true;
  }

  virtual ~OSXWindowImpl() {
    if (m_window) {
      [m_window removeImpl];
      [(OSXView*)m_window.contentView destroyMouseTrackingArea];

      // Select other window
      {
        auto app = [NSApplication sharedApplication];
        auto index = [app.windows indexOfObject:m_window];
        if (index+1 < app.windows.count) {
          ++index;
        }
        else {
          --index;
        }
        if (index >= 0 && index < app.windows.count)
          [[app.windows objectAtIndex:index] makeKeyWindow];
      }

      [m_window discardEventsMatchingMask:NSEventMaskAny
                              beforeEvent:nullptr];
      [m_window close];
      m_window = nil;
    }
  }

  gfx::Size clientSize() const {
    return [m_window clientSize];
  }

  gfx::Size restoredSize() const {
    return [m_window restoredSize];
  }

  gfx::Rect frame() const {
    NSRect r = m_window.frame;
    return gfx::Rect(r.origin.x,
                     m_window.screen.frame.size.height - r.origin.y - r.size.height,
                     r.size.width, r.size.height);
  }

  gfx::Rect contentRect() const {
    NSRect r = [m_window contentRectForFrameRect:m_window.frame];
    return gfx::Rect(r.origin.x,
                     m_window.screen.frame.size.height - r.origin.y - r.size.height,
                     r.size.width, r.size.height);
  }

  os::ScreenRef screen() const {
    ASSERT(m_window);
    return os::make_ref<os::OSXScreen>(m_window.screen);
  }

  os::ColorSpaceRef colorSpace() const {
    if (auto defaultCS = os::instance()->displaysColorSpace())
      return defaultCS;

    ASSERT(m_window);
    return os::convert_nscolorspace_to_os_colorspace([m_window colorSpace]);
  }

  int scale() const {
    return [m_window scale];
  }

  void setScale(int scale) {
    [m_window setScale:scale];
  }

  void queueEvent(os::Event& ev) {
    onQueueEvent(ev);
  }

  virtual void onQueueEvent(os::Event& ev) = 0;
  virtual void onClose() = 0;
  virtual void onResize(const gfx::Size& size) = 0;
  virtual void onDrawRect(const gfx::Rect& rect) = 0;
  virtual void onWindowChanged() = 0;
  virtual void onStartResizing() = 0;
  virtual void onResizing(gfx::Size& size) = 0;
  virtual void onEndResizing() = 0;

  // This generally happens when the window is moved to another
  // monitor with different scale (e.g. Retina vs non-Retina display)
  // or when the color space changes.
  virtual void onChangeBackingProperties() = 0;

protected:
  OSXWindow* __weak m_window = nullptr;
};

#endif
