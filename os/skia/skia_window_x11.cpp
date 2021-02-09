// LAF OS Library
// Copyright (C) 2020-2021  Igara Studio S.A.
// Copyright (C) 2016-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/skia/skia_window_x11.h"

#include "gfx/size.h"
#include "os/event.h"
#include "os/event_queue.h"
#include "os/skia/skia_surface.h"
#include "os/skia/skia_window.h"
#include "os/system.h"
#include "os/x11/x11.h"

#include "SkBitmap.h"

namespace os {

namespace {

bool convert_skia_bitmap_to_ximage(const SkBitmap& bitmap, XImage& image)
{
  memset(&image, 0, sizeof(image));
  int bpp = 8*bitmap.bytesPerPixel();
  image.width = bitmap.width();
  image.height = bitmap.height();
  image.format = ZPixmap;
  image.data = (char*)bitmap.getPixels();
  image.byte_order = LSBFirst;
  image.bitmap_unit = bpp;
  image.bitmap_bit_order = LSBFirst;
  image.bitmap_pad = bpp;
  image.depth = 24;
  image.bytes_per_line = bitmap.rowBytes() - 4*bitmap.width();
  image.bits_per_pixel = bpp;

  return (XInitImage(&image) ? true: false);
}

} // anonymous namespace

SkiaWindowX11::SkiaWindowX11(EventQueue* queue,
                             const WindowSpec& spec)
  : SkiaWindowBase<WindowX11>(X11::instance()->display(), spec)
  , m_queue(queue)
{
  initColorSpace();
}

void SkiaWindowX11::onQueueEvent(Event& ev)
{
  ev.setWindow(AddRef(this));
  m_queue->queueEvent(ev);
}

void SkiaWindowX11::onPaint(const gfx::Rect& rc)
{
  auto surface = static_cast<SkiaSurface*>(this->surface());
  const SkBitmap& bitmap = surface->bitmap();

  int scale = this->scale();
  if (scale == 1) {
    XImage image;
    if (convert_skia_bitmap_to_ximage(bitmap, image)) {
      XPutImage(
        x11display(), x11window(), gc(), &image,
        rc.x, rc.y,
        rc.x, rc.y,
        rc.w, rc.h);
    }
  }
  else {
    SkBitmap scaled;
    const SkImageInfo info =
      SkImageInfo::Make(rc.w, rc.h,
                        bitmap.info().colorType(),
                        bitmap.info().alphaType());

    // Increase m_buffer for "scaled" pixels if needed.
    const size_t rowBytes = info.minRowBytes();
    const size_t requiredSize = info.computeByteSize(rowBytes);
    if (requiredSize > m_buffer.size())
      m_buffer.resize(requiredSize);

    if (scaled.installPixels(info, (void*)&m_buffer[0], rowBytes)) {
      SkPaint paint;
      paint.setBlendMode(SkBlendMode::kSrc);

      SkCanvas canvas(scaled);
      SkRect srcRect = SkRect::Make(SkIRect::MakeXYWH(rc.x/scale, rc.y/scale, rc.w/scale, rc.h/scale));
      SkRect dstRect = SkRect::Make(SkIRect::MakeXYWH(0, 0, rc.w, rc.h));
      canvas.drawBitmapRect(bitmap, srcRect, dstRect, &paint,
                            SkCanvas::kStrict_SrcRectConstraint);

      XImage image;
      if (convert_skia_bitmap_to_ximage(scaled, image)) {
        XPutImage(
          x11display(), x11window(), gc(), &image,
          0, 0,
          rc.x, rc.y,
          rc.w, rc.h);
      }
    }
  }
}

void SkiaWindowX11::onResize(const gfx::Size& sz)
{
  resizeSkiaSurface(sz);
  if (os::instance()->handleWindowResize &&
      // Check that the surface is created to avoid a call to
      // handleWindowResize() with an empty surface (or null
      // SkiaSurface::m_canvas) when the window is being created.
      isInitialized()) {
    os::instance()->handleWindowResize(this);
  }
  else {
    Event ev;
    ev.setType(Event::ResizeWindow);
    ev.setWindow(AddRef(this));
    queue_event(ev);
  }
}

} // namespace os
