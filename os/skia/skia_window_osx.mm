// LAF OS Library
// Copyright (C) 2018-2021  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

//#define DEBUG_UPDATE_RECTS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/skia/skia_window_osx.h"

#include "base/log.h"
#include "gfx/region.h"
#include "gfx/size.h"
#include "os/event.h"
#include "os/event_queue.h"
#include "os/osx/event_queue.h"
#include "os/osx/view.h"
#include "os/skia/skia_color_space.h"
#include "os/skia/skia_surface.h"
#include "os/skia/skia_window_osx.h"
#include "os/system.h"

#include "mac/SkCGUtils.h"

#if SK_SUPPORT_GPU

  #include "GrBackendSurface.h"
  #include "GrContext.h"
  #include "gl/GrGLDefines.h"
  #include "gl/GrGLInterface.h"
  #include "os/skia/skia_surface.h"
  #include <OpenGL/gl.h>

#endif

#include <algorithm>
#include <iostream>

namespace os {

SkiaWindowOSX::SkiaWindowOSX(EventQueue* queue,
                             const WindowSpec& spec)
#if SK_SUPPORT_GPU
  : m_nsGL(nil)
  , m_nsPixelFormat(nil)
  , m_skSurface(nullptr)
#endif
{
  m_closing = false;
  createWindow(spec);
  initColorSpace();
}

SkiaWindowOSX::~SkiaWindowOSX()
{
#if SK_SUPPORT_GPU
  if (m_backend == Backend::GL)
    detachGL();
#endif
  destroyWindow();
}

void SkiaWindowOSX::setFullscreen(bool state)
{
  // Do not call toggleFullScreen in the middle of other toggleFullScreen
  if (m_resizingCount > 0)
    return;

  WindowOSX::setFullscreen(state);
}

void SkiaWindowOSX::invalidateRegion(const gfx::Region& rgn)
{
  switch (m_backend) {

    case Backend::NONE:
      @autoreleasepool {
        gfx::Rect bounds = rgn.bounds(); // TODO use only the region?
        int scale = this->scale();
        NSView* view = m_nsWindow.contentView;
        [view setNeedsDisplayInRect:
                NSMakeRect(bounds.x*scale,
                           view.frame.size.height - (bounds.y+bounds.h)*scale,
                           bounds.w*scale,
                           bounds.h*scale)];

#if 0     // Do not refresh immediately. Note: This might be required
          // for debugging purposes in some scenarios, but now this is
          // not required in release mode.
          //
          // TODO maybe in a future we could add an Display::update()
          //      or Display::refresh() member function
        [view displayIfNeeded];
#endif
      }
      break;

#if SK_SUPPORT_GPU
    case Backend::GL:
      if (m_skSurfaceDirect) {
        if (m_skSurface != m_skSurfaceDirect) {
          SkPaint paint;
          paint.setBlendMode(SkBlendMode::kSrc);
          sk_sp<SkImage> snapshot = m_skSurface->makeImageSnapshot();

          m_skSurfaceDirect->getCanvas()->drawImageRect(
            snapshot,
            SkRect::Make(SkIRect::MakeXYWH(0, 0, snapshot->width(), snapshot->height())),
            SkRect::Make(SkIRect::MakeXYWH(0, 0, m_skSurfaceDirect->width(), m_skSurfaceDirect->height())),
            &paint, SkCanvas::kStrict_SrcRectConstraint);
        }
        // We have use SkSurface::flush() explicitly because we are
        // going to call a native OpenGL function. Skia calls
        // flush() automatically only when you interact with their
        // API, but when you need to call the native API manually,
        // it's necessary to call it explicitly.
        m_skSurfaceDirect->flush();
      }
      if (m_nsGL) {
        // Flush all commands and swap front/back buffers (this will
        // make the back buffer visible to the user)
        [m_nsGL flushBuffer];

        // Copy the front buffer to the back buffer. This is because
        // in the non-GPU backend the back buffer persists between
        // each flip: each frame is not drawn from scratch, so we
        // can draw only the differences. In this way we keep both
        // buffers in sync to support this kind of behavior.
        //
        // TODO make this configurable in case that we want to draw
        //      each frame from scratch, e.g. game-like frame rendering
        if (m_skSurfaceDirect) {
          const int w = m_skSurfaceDirect->width();
          const int h = m_skSurfaceDirect->height();
          glReadBuffer(GL_FRONT);
          glDrawBuffer(GL_BACK);
          glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
      }
      break;
#endif

  }
}

void SkiaWindowOSX::setTranslateDeadKeys(bool state)
{
  ViewOSX* view = (ViewOSX*)m_nsWindow.contentView;
  [view setTranslateDeadKeys:(state ? YES: NO)];
}

void SkiaWindowOSX::onQueueEvent(Event& ev)
{
  ev.setWindow(AddRef(this));
  os::queue_event(ev);
}

void SkiaWindowOSX::onClose()
{
  m_closing = true;
}

void SkiaWindowOSX::onResize(const gfx::Size& size)
{
  bool gpu = os::instance()->gpuAcceleration();
  (void)gpu;

#if SK_SUPPORT_GPU
  if (gpu && attachGL()) {
    m_backend = Backend::GL;
  }
  else
#endif
  {
#if SK_SUPPORT_GPU
    detachGL();
#endif
    m_backend = Backend::NONE;
  }

#if SK_SUPPORT_GPU
  if (m_nsGL && this->isInitialized())
    createRenderTarget(size);
#endif

  this->resizeSkiaSurface(size);
  if (m_backend == Backend::GL)
    invalidateRegion(gfx::Region(gfx::Rect(size)));
}

void SkiaWindowOSX::onDrawRect(const gfx::Rect& rect)
{
  if (m_nsWindow.contentView.inLiveResize) {
    if (this->handleResize)
      this->handleResize(this);
  }

  switch (m_backend) {

    case Backend::NONE:
      paintGC(rect);
      break;

#if SK_SUPPORT_GPU
    case Backend::GL:
      // Do nothing
      break;
#endif
  }
}

void SkiaWindowOSX::onWindowChanged()
{
#if SK_SUPPORT_GPU
  if (m_nsGL)
    [m_nsGL setView:[m_nsWindow contentView]];
#endif
}

void SkiaWindowOSX::onStartResizing()
{
  if (++m_resizingCount > 1)
    return;
}

void SkiaWindowOSX::onResizing(gfx::Size& size)
{
  this->resizeSkiaSurface(size);
  if (this->handleResize) {
    this->handleResize(this);
  }
}

void SkiaWindowOSX::onEndResizing()
{
  if (--m_resizingCount > 0)
    return;

  // Generate the resizing display event for the user.
  if (!this->handleResize) {
    Event ev;
    ev.setType(Event::ResizeWindow);
    ev.setWindow(AddRef(this));
    os::queue_event(ev);
  }
}

void SkiaWindowOSX::onChangeBackingProperties()
{
  if (m_nsWindow)
    setColorSpace(colorSpace());
}

#if SK_SUPPORT_GPU
bool SkiaWindowOSX::attachGL()
{
  if (m_nsGL)
    return true;
  try {
    // set up pixel format
    std::vector<NSOpenGLPixelFormatAttribute> attr;
    attr.push_back(NSOpenGLPFAAccelerated);
    attr.push_back(NSOpenGLPFAClosestPolicy);
    attr.push_back(NSOpenGLPFADoubleBuffer);
    attr.push_back(NSOpenGLPFAOpenGLProfile);
    attr.push_back(NSOpenGLProfileVersion3_2Core);
    attr.push_back(NSOpenGLPFAColorSize);
    attr.push_back(24);
    attr.push_back(NSOpenGLPFAAlphaSize);
    attr.push_back(8);
    attr.push_back(NSOpenGLPFADepthSize);
    attr.push_back(0);
    attr.push_back(NSOpenGLPFAStencilSize);
    attr.push_back(8);
    attr.push_back(NSOpenGLPFASampleBuffers);
    attr.push_back(0);
    attr.push_back(0);

    m_nsPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:&attr[0]];
    if (nil == m_nsPixelFormat)
      return false;

    m_nsGL = [[NSOpenGLContext alloc] initWithFormat:m_nsPixelFormat
                                        shareContext:nil];
    if (!m_nsGL) {
      m_nsPixelFormat = nil;
      return false;
    }

    [m_nsGL setView:m_nsWindow.contentView];

    GLint swapInterval = 0;   // disable vsync
    [m_nsGL setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

    // TODO publish an API method to change this value to YES in
    // case that the user wants full control of the Retina pixels.
    [m_nsWindow.contentView setWantsBestResolutionOpenGLSurface:NO];

    [m_nsGL makeCurrentContext];

    m_glInterfaces.reset(GrGLCreateNativeInterface());
    if (!m_glInterfaces || !m_glInterfaces->validate()) {
      LOG(ERROR) << "OS: Cannot create GL interfaces\n";
      detachGL();
      return false;
    }

    m_grCtx = GrContext::MakeGL(m_glInterfaces);
    if (!m_grCtx) {
      LOG(ERROR) << "OS: Cannot create GrContext\n";
      detachGL();
      return false;
    }

    LOG("OS: Using OpenGL backend\n");

    createRenderTarget(clientSize());
  }
  catch (const std::exception& ex) {
    LOG(ERROR) << "OS: Cannot create GL context: " << ex.what() << "\n";
    detachGL();
    return false;
  }
  return true;
}

void SkiaWindowOSX::detachGL()
{
  if (m_nsGL) {
    LOG(INFO, "OS: detach GL context\n");
    m_nsGL = nil;
  }

  m_skSurface.reset(nullptr);
  m_skSurfaceDirect.reset(nullptr);
  m_grCtx.reset(nullptr);
}

void SkiaWindowOSX::createRenderTarget(const gfx::Size& size)
{
  [m_nsGL update];

  // Setup of a SkSurface connected to the framebuffer

  const int scale = this->scale();
  auto colorSpace = ((SkiaColorSpace*)this->colorSpace().get())->skColorSpace();

  GrGLint buffer;
  m_glInterfaces->fFunctions.fGetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer);

  GrGLFramebufferInfo fbInfo;
  fbInfo.fFBOID = buffer;
  fbInfo.fFormat = GR_GL_RGBA8;

  SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
                       SkSurfaceProps::kLegacyFontHost_InitType);

  GLint stencilBits;
  [m_nsGL.pixelFormat getValues:&stencilBits
                   forAttribute:NSOpenGLPFAStencilSize
               forVirtualScreen:0];

  GLint sampleCount;
  [m_nsGL.pixelFormat getValues:&sampleCount
                   forAttribute:NSOpenGLPFASamples
               forVirtualScreen:0];
  sampleCount = std::max(sampleCount, 1);

  GrBackendRenderTarget backendRT(size.w, size.h,
                                  sampleCount,
                                  stencilBits,
                                  fbInfo);

  m_skSurface.reset(nullptr); // set m_skSurface comparing with the old m_skSurfaceDirect
  m_skSurfaceDirect =
    SkSurface::MakeFromBackendRenderTarget(
      m_grCtx.get(), backendRT,
      kBottomLeft_GrSurfaceOrigin,
      kRGBA_8888_SkColorType,
      colorSpace,
      &props);

  if (scale == 1 && m_skSurfaceDirect) {
    LOG("OS: Using GL direct surface %p\n", m_skSurfaceDirect.get());
    m_skSurface = m_skSurfaceDirect;
  }
  else {
    LOG("OS: Using double buffering\n");

    SkImageInfo info = SkImageInfo::Make(
      std::max(1, size.w / scale),
      std::max(1, size.h / scale),
      kN32_SkColorType,
      kOpaque_SkAlphaType,
      colorSpace);

    m_skSurface =
      SkSurface::MakeRenderTarget(
        m_grCtx.get(), SkBudgeted::kNo,
        info, sampleCount, &props);
  }

  if (!m_skSurface)
    throw std::runtime_error("Error creating surface for main display");

  this->setSkiaSurface(new SkiaSurface(m_skSurface));
}

#endif

void SkiaWindowOSX::paintGC(const gfx::Rect& rect)
{
  if (!this->isInitialized())
    return;

  if (rect.isEmpty())
    return;

  NSRect viewBounds = m_nsWindow.contentView.bounds;
  int scale = this->scale();

  SkiaSurface* surface = static_cast<SkiaSurface*>(this->surface());
  if (!surface->isValid())
    return;

  const SkBitmap& origBitmap = surface->bitmap();

  SkBitmap bitmap;
  if (scale == 1) {
    // Create a subset to draw on the view
    if (!origBitmap.extractSubset(
          &bitmap, SkIRect::MakeXYWH(rect.x,
                                     (viewBounds.size.height-(rect.y+rect.h)),
                                     rect.w,
                                     rect.h)))
      return;
  }
  else {
    // Create a bitmap to draw the original one scaled. This is
    // faster than doing the scaling directly in
    // CGContextDrawImage(). This avoid a slow path where the
    // internal macOS argb32_image_mark_RGB32() function is called
    // (which is a performance hit).
    if (!bitmap.tryAllocN32Pixels(rect.w, rect.h, true))
      return;

    SkCanvas canvas(bitmap);
    canvas.drawBitmapRect(origBitmap,
                          SkIRect::MakeXYWH(rect.x/scale,
                                            (viewBounds.size.height-(rect.y+rect.h))/scale,
                                            rect.w/scale,
                                            rect.h/scale),
                          SkRect::MakeXYWH(0, 0, rect.w, rect.h),
                          nullptr);
  }

  @autoreleasepool {
    NSGraphicsContext* gc = [NSGraphicsContext currentContext];
    CGContextRef cg = (CGContextRef)[gc graphicsPort];
    // TODO we can be in other displays (non-main display)
    CGColorSpaceRef colorSpace = CGDisplayCopyColorSpace(CGMainDisplayID());
    CGImageRef img = SkCreateCGImageRefWithColorspace(bitmap, colorSpace);
    if (img) {
      CGRect r = CGRectMake(viewBounds.origin.x+rect.x,
                            viewBounds.origin.y+rect.y,
                            rect.w, rect.h);

      CGContextSaveGState(cg);
      CGContextSetInterpolationQuality(cg, kCGInterpolationNone);
      CGContextDrawImage(cg, r, img);
#ifdef DEBUG_UPDATE_RECTS
      {
        static int i = 0;
        i = (i+1) % 8;
        CGContextSetRGBStrokeColor(cg,
                                   (i & 1 ? 1.0f: 0.0f),
                                   (i & 2 ? 1.0f: 0.0f),
                                   (i & 4 ? 1.0f: 0.0f), 1.0f);
        CGContextStrokeRectWithWidth(cg, r, 2.0f);
      }
#endif
      CGContextRestoreGState(cg);
      CGImageRelease(img);
    }
    CGColorSpaceRelease(colorSpace);
  }
}

} // namespace os
