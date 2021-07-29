// LAF OS Library
// Copyright (C) 2019-2021  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/skia/skia_window_win.h"

#include "base/log.h"
#include "gfx/region.h"
#include "os/event.h"
#include "os/event_queue.h"
#include "os/skia/skia_window.h"
#include "os/system.h"

#if SK_SUPPORT_GPU

  #include "GrBackendSurface.h"
  #include "GrContext.h"
  #include "gl/GrGLDefines.h"
  #include "os/gl/gl_context_wgl.h"
  #if SK_ANGLE
    #include "os/gl/gl_context_egl.h"
    #include "gl/GrGLAssembleInterface.h"
  #endif

#endif

#include <windows.h>
#include "os/win/window_dde.h"

#include <algorithm>
#include <iostream>

namespace os {

SkiaWindowWin::SkiaWindowWin(const WindowSpec& spec)
  : SkiaWindowBase<WindowWin>(spec)
  , m_backend(Backend::NONE)
#if SK_SUPPORT_GPU
  , m_skSurface(nullptr)
  , m_sampleCount(0)
  , m_stencilBits(0)
#endif
{
  initColorSpace();
}

SkiaWindowWin::~SkiaWindowWin()
{
  switch (m_backend) {

    case Backend::NONE:
      // Do nothing
      break;

#if SK_SUPPORT_GPU

    case Backend::GL:
    case Backend::ANGLE:
      detachGL();
      break;

#endif // SK_SUPPORT_GPU
  }
}

void SkiaWindowWin::onPaint(HDC hdc)
{
  switch (m_backend) {

    case Backend::NONE:
      paintHDC(hdc);
      break;

#if SK_SUPPORT_GPU

    case Backend::GL:
    case Backend::ANGLE:
      // Flush operations to the SkCanvas
      {
        SkiaSurface* surface = static_cast<SkiaSurface*>(this->getSurface());
        surface->flush();
      }

      // If we are drawing inside an off-screen texture, here we have
      // to blit that texture into the main framebuffer.
      if (m_skSurfaceDirect != m_skSurface) {
#if 0                           // TODO
        GrBackendObject texID = m_skSurface->getTextureHandle(
          SkSurface::kFlushRead_BackendHandleAccess);

        GrBackendTextureDesc texDesc;
        texDesc.fFlags = kNone_GrBackendTextureFlag;
        texDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
        texDesc.fWidth = m_lastSize.w / scale();
        texDesc.fHeight = m_lastSize.h / scale();
        texDesc.fConfig = kSkia8888_GrPixelConfig;
        texDesc.fSampleCnt = m_sampleCount;
        texDesc.fTextureHandle = texID;
        sk_sp<SkImage> image(SkImage::MakeFromTexture(m_grCtx.get(), texDesc));

        SkRect dstRect(SkRect::MakeWH(SkIntToScalar(m_lastSize.w),
                                      SkIntToScalar(m_lastSize.h)));

        SkPaint paint;
        m_skSurfaceDirect->getCanvas()->drawImageRect(
          image, dstRect, &paint,
          SkCanvas::kStrict_SrcRectConstraint);

        m_skSurfaceDirect->getCanvas()->flush();
#endif
      }

      // Flush GL context
      m_glInterfaces->fFunctions.fFlush();
      m_glCtx->swapBuffers();
      break;

#endif // SK_SUPPORT_GPU
  }
}

void SkiaWindowWin::invalidateRegion(const gfx::Region& rgn)
{
  if (!isTransparent())
    return WindowWin::invalidateRegion(rgn);

  // Special logic for transparent (WS_EX_LAYERED) windows: we call
  // UpdateLayeredWindowIndirect() because we want to present the RGBA
  // surface as the window surface with alpha per pixel.

  SkiaSurface* surface = static_cast<SkiaSurface*>(this->surface());
  ASSERT(surface);

  if (!surface || !surface->isValid())
    return;

  const SkBitmap& bitmap = surface->bitmap();
  const int w = bitmap.width();
  const int h = bitmap.height();
  const int s = scale();
  const int sw = bitmap.width()*s;
  const int sh = bitmap.height()*s;

  HWND hwnd = (HWND)nativeHandle();
  HDC hdc = GetDC(nullptr);
  HBITMAP hbmpScaled = CreateCompatibleBitmap(hdc, sw, sh);
  HBITMAP hbmp = CreateBitmap(w, h, 1, 32, (void*)bitmap.getPixels());
  HDC srcHdcScaled = CreateCompatibleDC(hdc);
  HDC srcHdc = CreateCompatibleDC(hdc);
  SelectObject(srcHdcScaled, hbmpScaled);
  SelectObject(srcHdc, hbmp);

  BLENDFUNCTION bf;
  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = 255;
  bf.AlphaFormat = AC_SRC_ALPHA;

  AlphaBlend(srcHdcScaled, 0, 0, sw, sh,
             srcHdc, 0, 0, w, h, bf);

  const gfx::Rect rect = frame();
  const POINT dstPoint = { rect.x, rect.y };
  const SIZE dstSize = { rect.w, rect.h };
  POINT srcPos = { 0, 0 };

  const gfx::Rect dirtyBounds = rgn.bounds();
  const RECT dirty = {
    s*dirtyBounds.x,
    s*dirtyBounds.y,
    s*dirtyBounds.x2(),
    s*dirtyBounds.y2()
  };

  UPDATELAYEREDWINDOWINFO ulwi;
  memset(&ulwi, 0, sizeof(ulwi));
  ulwi.cbSize = sizeof(ulwi);
  ulwi.hdcDst = hdc;
  ulwi.pptDst = &dstPoint;
  ulwi.psize = &dstSize;
  ulwi.hdcSrc = srcHdcScaled;
  ulwi.pptSrc = &srcPos;
  ulwi.pblend = &bf;
  ulwi.dwFlags = ULW_ALPHA;
  ulwi.prcDirty = &dirty;

  UpdateLayeredWindowIndirect(hwnd, &ulwi);

  ReleaseDC(nullptr, hdc);
  DeleteObject(hbmpScaled);
  DeleteObject(hbmp);
  DeleteDC(srcHdcScaled);
  DeleteDC(srcHdc);
}

void SkiaWindowWin::paintHDC(HDC hdc)
{
  if (isTransparent()) {
    // In transparent windows we don't handle WM_PAINT messages, but
    // call UpdateLayeredWindowIndirect() directly from
    // invalidateRegion().
    return;
  }

  SkiaSurface* surface = static_cast<SkiaSurface*>(this->surface());
  ASSERT(surface);

  // It looks like the surface can be nullptr here from a WM_PAINT
  // message (issue received from a dmp file).
  if (!surface || !surface->isValid())
    return;

  const SkBitmap& bitmap = surface->bitmap();

  BITMAPINFO bmi;
  memset(&bmi, 0, sizeof(bmi));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = bitmap.width();
  bmi.bmiHeader.biHeight = -bitmap.height();
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 0;

  ASSERT(bitmap.width() * bitmap.bytesPerPixel() == bitmap.rowBytes());

  int ret = StretchDIBits(hdc,
    0, 0, bitmap.width()*scale(), bitmap.height()*scale(),
    0, 0, bitmap.width(), bitmap.height(),
    bitmap.getPixels(),
    &bmi, DIB_RGB_COLORS, SRCCOPY);
  (void)ret;
}

#if SK_SUPPORT_GPU

#if SK_ANGLE

struct ANGLEAssembleContext {
  ANGLEAssembleContext() {
    fEGL = GetModuleHandle(L"libEGL.dll");
    fGL = GetModuleHandle(L"libGLESv2.dll");
  }

  bool isValid() const { return SkToBool(fEGL) && SkToBool(fGL); }

  HMODULE fEGL;
  HMODULE fGL;
};

static GrGLFuncPtr angle_get_gl_proc(void* ctx, const char name[]) {
  const ANGLEAssembleContext& context = *reinterpret_cast<const ANGLEAssembleContext*>(ctx);
  GrGLFuncPtr proc = (GrGLFuncPtr) GetProcAddress(context.fGL, name);
  if (proc) {
    return proc;
  }
  proc = (GrGLFuncPtr) GetProcAddress(context.fEGL, name);
  if (proc) {
    return proc;
  }
  return eglGetProcAddress(name);
}

static const GrGLInterface* get_angle_gl_interface() {
  ANGLEAssembleContext context;
  if (!context.isValid()) {
    return nullptr;
  }
  return GrGLAssembleGLESInterface(&context, angle_get_gl_proc);
}

bool SkiaWindowWin::attachANGLE()
{
  if (!m_glCtx) {
    try {
      std::unique_ptr<GLContext> ctx(new GLContextEGL(handle()));
      if (!ctx->createGLContext())
        throw std::runtime_error("Cannot create EGL context");

      m_glInterfaces.reset(get_angle_gl_interface());
      if (!m_glInterfaces || !m_glInterfaces->validate())
        throw std::runtime_error("Cannot create EGL interfaces\n");

      m_stencilBits = ctx->getStencilBits();
      m_sampleCount = ctx->getSampleCount();

      m_glCtx.reset(ctx.release());
      m_grCtx.reset(
        GrContext::Create(kOpenGL_GrBackend,
                          (GrBackendContext)m_glInterfaces.get()));

      LOG("OS: Using EGL backend\n");
    }
    catch (const std::exception& ex) {
      LOG(ERROR) << "OS: Error initializing EGL backend: " << ex.what() << "\n";
      detachGL();
    }
  }

  if (m_glCtx)
    return true;
  else
    return false;
}

#endif // SK_ANGLE

bool SkiaWindowWin::attachGL()
{
  if (!m_glCtx) {
    try {
      std::unique_ptr<GLContext> ctx(new GLContextWGL(handle()));
      if (!ctx->createGLContext())
        throw std::runtime_error("Cannot create WGL context\n");

      m_glInterfaces.reset(GrGLCreateNativeInterface());
      if (!m_glInterfaces || !m_glInterfaces->validate())
        throw std::runtime_error("Cannot create WGL interfaces\n");

      m_stencilBits = ctx->getStencilBits();
      m_sampleCount = ctx->getSampleCount();

      m_glCtx.reset(ctx.release());
      m_grCtx.reset(
        GrContext::Create(kOpenGL_GrBackend,
                          (GrBackendContext)m_glInterfaces.get()));

      LOG("OS: Using WGL backend\n");
    }
    catch (const std::exception& ex) {
      LOG(ERROR) << "OS: Error initializing WGL backend: " << ex.what() << "\n";
      detachGL();
    }
  }

  if (m_glCtx)
    return true;
  else
    return false;
}

void SkiaWindowWin::detachGL()
{
  if (m_glCtx)
    this->resetSkiaSurface();

  m_skSurface.reset(nullptr);
  m_skSurfaceDirect.reset(nullptr);
  m_grCtx.reset(nullptr);
  m_glCtx.reset(nullptr);
}

void SkiaWindowWin::createRenderTarget(const gfx::Size& size)
{
  int scale = this->scale();
  m_lastSize = size;

  GrGLint buffer;
  m_glInterfaces->fFunctions.fGetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &buffer);
  GrGLFramebufferInfo info;
  info.fFBOID = (GrGLuint) buffer;

  GrBackendRenderTarget
    target(size.w, size.h,
           m_sampleCount,
           m_stencilBits,
           kSkia8888_GrPixelConfig,
           info);

  SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

  m_skSurface.reset(nullptr); // set m_skSurface comparing with the old m_skSurfaceDirect
  m_skSurfaceDirect = SkSurface::MakeFromBackendRenderTarget(
    m_grCtx.get(), target,
    kBottomLeft_GrSurfaceOrigin,
    nullptr, &props);

  if (scale == 1) {
    m_skSurface = m_skSurfaceDirect;
  }
  else {
    m_skSurface =
      SkSurface::MakeRenderTarget(
        m_grCtx.get(),
        SkBudgeted::kYes,
        SkImageInfo::MakeN32Premul(std::max(1, size.w / scale),
                                   std::max(1, size.h / scale)),
        m_sampleCount,
        nullptr);
  }

  if (!m_skSurface)
    throw std::runtime_error("Error creating surface for main display");

  this->setSkiaSurface(new SkiaSurface(m_skSurface));
}

#endif // SK_SUPPORT_GPU

void SkiaWindowWin::onResize(const gfx::Size& size)
{
  bool gpu = instance()->gpuAcceleration();
  (void)gpu;

#if SK_SUPPORT_GPU
#if SK_ANGLE
  if (gpu && attachANGLE()) {
    m_backend = Backend::ANGLE;
  }
  else
#endif // SK_ANGLE
  if (gpu && attachGL()) {
    m_backend = Backend::GL;
  }
  else
#endif // SK_SUPPORT_GPU
  {
#if SK_SUPPORT_GPU
    detachGL();
#endif
    m_backend = Backend::NONE;
  }

#if SK_SUPPORT_GPU
  if (m_glCtx)
    createRenderTarget(size);
#endif

  // TODO the next code is quite similar to SkiaWindow::onResize() for X11.

  this->resizeSkiaSurface(size);
  if (os::instance()->handleWindowResize &&
      // Check that the surface is created to avoid a call to
      // handleWindowResize() with an empty surface (or null
      // SkiaSurface::m_canvas) when the window is being created.
      isInitialized()) {
    os::instance()->handleWindowResize(this);
  }
  else if (!m_resizing) {
    Event ev;
    ev.setType(Event::ResizeWindow);
    ev.setWindow(AddRef(this));
    queue_event(ev);
  }
}

void SkiaWindowWin::onStartResizing()
{
  m_resizing = true;
}

void SkiaWindowWin::onEndResizing()
{
  m_resizing = false;

  Event ev;
  ev.setType(Event::ResizeWindow);
  ev.setWindow(AddRef(this));
  queue_event(ev);
}

void SkiaWindowWin::onChangeColorSpace()
{
  this->setColorSpace(colorSpace());
}

} // namespace os
