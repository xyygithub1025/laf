// LAF OS Library
// Copyright (C) 2021  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_SKIA_SKIA_WINDOW_BASE_INCLUDED
#define OS_SKIA_SKIA_WINDOW_BASE_INCLUDED
#pragma once

#include "os/event.h"
#include "os/event_queue.h"
#include "os/skia/skia_surface.h"
#include "os/window.h"

namespace os {

template<typename T>
class SkiaWindowBase : public T {
public:
  template<typename... Args>
  SkiaWindowBase(Args&&... args)
    : T(std::forward<Args&&>(args)...)
    , m_initialized(false)
    , m_surface(new SkiaSurface)
    , m_colorSpace(nullptr)
    , m_customSurface(false) {
  }

  void initColorSpace() {
    // Needed on macOS because WindowOSX::colorSpace() needs the
    // m_nsWindow created, and that happens after
    // WindowOSX::createWindow() is called.
    m_colorSpace = T::colorSpace();
  }

  bool isInitialized() const {
    return m_initialized;
  }

  void setSkiaSurface(SkiaSurface* surface) {
    m_surface = AddRef(surface);
    m_customSurface = true;
  }

  void resetSkiaSurface() {
    if (m_surface)
      m_surface = nullptr;

    m_customSurface = false;
    resizeSkiaSurface(this->clientSize());
  }

  void resizeSkiaSurface(const gfx::Size& size) {
    if (!m_initialized || m_customSurface)
      return;

    gfx::Size newSize(size.w / this->scale(),
                      size.h / this->scale());
    newSize.w = std::max(1, newSize.w);
    newSize.h = std::max(1, newSize.h);

    if (m_initialized &&
        m_surface &&
        m_surface->width() == newSize.w &&
        m_surface->height() == newSize.h) {
      return;
    }

    if (!m_surface)
      m_surface = make_ref<SkiaSurface>();
    m_surface->create(newSize.w, newSize.h, m_colorSpace);
  }


  // Returns the main surface to draw into this window.
  // You must not dispose this surface.
  Surface* surface() override {
    return m_surface.get();
  }

  // Overrides the colorSpace() method to return the cached/stored
  // color space in this instance (instead of asking for the color
  // space to the screen as T::colorSpace() should do).
  os::ColorSpaceRef colorSpace() const override {
    return m_colorSpace;
  }

  void setColorSpace(const os::ColorSpaceRef& colorSpace) {
    ASSERT(colorSpace);
    m_colorSpace = colorSpace;
    if (m_surface)
      resetSkiaSurface();

    // Generate the resizing window event to redraw everything.
    // TODO we could create a new event like Event::ColorSpaceChange,
    // but the result would be the same, the window must be re-painted.
    Event ev;
    ev.setType(Event::ResizeWindow);
    ev.setWindow(AddRef(this));
    os::queue_event(ev);
  }

protected:
  void initializeSurface() {
    m_initialized = true;
    resetSkiaSurface();
  }

private:
  // Flag used to avoid accessing to an invalid m_surface in the first
  // SkiaWindow::resize() call when the window is created (as the
  // window is created, it send a first resize event.)
  bool m_initialized;
  Ref<SkiaSurface> m_surface;
  os::ColorSpaceRef m_colorSpace;
  bool m_customSurface;
};

} // namespace os

#endif
