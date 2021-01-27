// LAF OS Library
// Copyright (C) 2018-2021  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/skia/skia_display.h"

#include "base/debug.h"
#include "os/display_spec.h"
#include "os/event.h"
#include "os/event_queue.h"
#include "os/skia/skia_surface.h"
#include "os/system.h"

namespace os {

SkiaDisplay::SkiaDisplay(const DisplaySpec& spec)
  : m_initialized(false)
  , m_window(instance()->eventQueue(), this, spec)
  , m_surface(new SkiaSurface)
  , m_colorSpace(m_window.colorSpace())
  , m_customSurface(false)
  , m_nativeCursor(kArrowCursor)
{
  m_window.setScale(spec.scale());
  m_window.setVisible(true);

  m_initialized = true;
  resetSkiaSurface();
}

void SkiaDisplay::setSkiaSurface(SkiaSurface* surface)
{
  m_surface = AddRef(surface);
  m_customSurface = true;
}

void SkiaDisplay::resetSkiaSurface()
{
  if (m_surface)
    m_surface = nullptr;

  m_customSurface = false;
  resizeSkiaSurface(m_window.clientSize());
}

void SkiaDisplay::resizeSkiaSurface(const gfx::Size& size)
{
  if (!m_initialized || m_customSurface)
    return;

  gfx::Size newSize(size.w / m_window.scale(),
                    size.h / m_window.scale());
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

gfx::Rect SkiaDisplay::frame() const
{
  return m_window.frame();
}

gfx::Rect SkiaDisplay::contentRect() const
{
  return m_window.contentRect();
}

int SkiaDisplay::width() const
{
  return m_window.clientSize().w;
}

int SkiaDisplay::height() const
{
  return m_window.clientSize().h;
}

int SkiaDisplay::originalWidth() const
{
  return m_window.restoredSize().w;
}

int SkiaDisplay::originalHeight() const
{
  return m_window.restoredSize().h;
}

int SkiaDisplay::scale() const
{
  return m_window.scale();
}

void SkiaDisplay::setScale(int scale)
{
  ASSERT(scale > 0);
  m_window.setScale(scale);
}

bool SkiaDisplay::isVisible() const
{
  return m_window.isVisible();
}

void SkiaDisplay::setVisible(bool visible)
{
  m_window.setVisible(visible);
}

Surface* SkiaDisplay::surface()
{
  return m_surface.get();
}

// Flips all graphics in the surface to the real display.  Returns
// false if the flip couldn't be done because the display was
// resized.
void SkiaDisplay::invalidateRegion(const gfx::Region& rgn)
{
  m_window.invalidateRegion(rgn);
}

void SkiaDisplay::maximize()
{
  m_window.maximize();
}

bool SkiaDisplay::isMaximized() const
{
  return m_window.isMaximized();
}

bool SkiaDisplay::isMinimized() const
{
  return m_window.isMinimized();
}

bool SkiaDisplay::isFullscreen() const
{
  return m_window.isFullscreen();
}

void SkiaDisplay::setFullscreen(bool state)
{
  m_window.setFullscreen(state);
}

std::string SkiaDisplay::title() const
{
  return m_window.title();
}

void SkiaDisplay::setTitle(const std::string& title)
{
  m_window.setTitle(title);
}

void SkiaDisplay::setIcons(const SurfaceList& icons)
{
#if !defined(_WIN32) && !defined(__APPLE__)
  m_window.setIcons(icons);
#endif
}

NativeCursor SkiaDisplay::nativeMouseCursor()
{
  return m_nativeCursor;
}

bool SkiaDisplay::setNativeMouseCursor(NativeCursor cursor)
{
  m_nativeCursor = cursor;
  return m_window.setNativeMouseCursor(cursor);
}

bool SkiaDisplay::setNativeMouseCursor(const os::Surface* surface,
                                       const gfx::Point& focus,
                                       const int scale)
{
  return m_window.setNativeMouseCursor(surface, focus, scale);
}

void SkiaDisplay::setMousePosition(const gfx::Point& position)
{
  m_window.setMousePosition(position);
}

void SkiaDisplay::captureMouse()
{
  m_window.captureMouse();
}

void SkiaDisplay::releaseMouse()
{
  m_window.releaseMouse();
}

std::string SkiaDisplay::getLayout()
{
  return m_window.getLayout();
}

void SkiaDisplay::setLayout(const std::string& layout)
{
  m_window.setLayout(layout);
}

void SkiaDisplay::setInterpretOneFingerGestureAsMouseMovement(bool state)
{
#ifdef _WIN32
  m_window.setInterpretOneFingerGestureAsMouseMovement(state);
#endif
}

void SkiaDisplay::setTranslateDeadKeys(bool state)
{
  m_window.setTranslateDeadKeys(state);
}

void SkiaDisplay::setColorSpace(const os::ColorSpaceRef& colorSpace)
{
  ASSERT(colorSpace);
  m_colorSpace = colorSpace;
  if (m_surface)
    resetSkiaSurface();

  // Generate the resizing display event to redraw everything.
  // TODO we could create a new event like Event::ColorSpaceChange,
  // but the result would be the same, the display must be re-painted.
  Event ev;
  ev.setType(Event::ResizeDisplay);
  ev.setDisplay(AddRef(this));
  os::queue_event(ev);

  TRACE("SkiaDisplay::setColorSpace %s\n",
        colorSpace ? colorSpace->gfxColorSpace()->name().c_str():
                     "nullptr");
}

os::ScreenRef SkiaDisplay::screen() const
{
  return m_window.screen();
}

os::ColorSpaceRef SkiaDisplay::currentMonitorColorSpace() const
{
  return m_window.colorSpace();
}

void SkiaDisplay::onTabletAPIChange()
{
#if _WIN32
  m_window.onTabletAPIChange();
#endif
}

Display::NativeHandle SkiaDisplay::nativeHandle()
{
  return (NativeHandle)m_window.handle();
}

} // namespace os
