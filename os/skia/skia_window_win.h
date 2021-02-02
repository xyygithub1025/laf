// LAF OS Library
// Copyright (C) 2021  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_SKIA_SKIA_WINDOW_WIN_INCLUDED
#define OS_SKIA_SKIA_WINDOW_WIN_INCLUDED
#pragma once

#include "base/disable_copying.h"
#include "os/skia/skia_window_base.h"
#include "os/win/window.h"

#if SK_SUPPORT_GPU
  #include "gl/GrGLInterface.h"
  #include "os/gl/gl_context.h"
#endif

namespace os {

class EventQueue;

class SkiaWindowWin : public SkiaWindowBase<WindowWin> {
public:
  enum class Backend { NONE, GL, ANGLE };

  SkiaWindowWin(EventQueue* queue,
                const WindowSpec& spec);
  ~SkiaWindowWin();

private:
  void onQueueEvent(Event& ev) override;
  void onPaint(HDC hdc) override;
  void onResize(const gfx::Size& sz) override;
  void onStartResizing() override;
  void onEndResizing() override;
  void onChangeColorSpace() override;
  void paintHDC(HDC dc);

#if SK_SUPPORT_GPU
#if SK_ANGLE
  bool attachANGLE();
#endif // SK_ANGLE
  bool attachGL();
  void detachGL();
  void createRenderTarget(const gfx::Size& size);
#endif // SK_SUPPORT_GPU

  EventQueue* m_queue;
  Backend m_backend;
  bool m_resizing = false;
#if SK_SUPPORT_GPU
  std::unique_ptr<GLContext> m_glCtx;
  sk_sp<const GrGLInterface> m_glInterfaces;
  sk_sp<GrContext> m_grCtx;
  sk_sp<SkSurface> m_skSurfaceDirect;
  sk_sp<SkSurface> m_skSurface;
  int m_sampleCount;
  int m_stencilBits;
  gfx::Size m_lastSize;
#endif // SK_SUPPORT_GPU

  DISABLE_COPYING(SkiaWindowWin);
};

} // namespace os

#endif
