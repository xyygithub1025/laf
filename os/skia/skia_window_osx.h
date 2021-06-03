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
#include "os/osx/window.h"
#include "os/screen.h"
#include "os/skia/skia_window_base.h"

#include <string>

namespace os {

class SkiaDisplay;
class Surface;
class WindowSpec;

class SkiaWindowOSX : public SkiaWindowBase<WindowOSX> {
public:
  enum class Backend { NONE, GL };

  SkiaWindowOSX(const WindowSpec& spec);
  ~SkiaWindowOSX();

  void setFullscreen(bool state) override;

  void invalidateRegion(const gfx::Region& rgn) override;
  std::string getLayout() override { return ""; }
  void setLayout(const std::string& layout) override { }
  void setTranslateDeadKeys(bool state);

  // WindowOSX overrides
  void onClose() override;
  void onResize(const gfx::Size& size) override;
  void onDrawRect(const gfx::Rect& rect) override;
  void onWindowChanged() override;
  void onStartResizing() override;
  void onResizing(gfx::Size& size) override;
  void onEndResizing() override;
  void onChangeBackingProperties() override;

private:
#if SK_SUPPORT_GPU
  bool attachGL();
  void detachGL();
  void createRenderTarget(const gfx::Size& size);
#endif // SK_SUPPORT_GPU
  void paintGC(const gfx::Rect& rect);

  Backend m_backend = Backend::NONE;
  bool m_closing = false;

  // Counter used to match each onStart/EndResizing() call because we
  // can receive multiple calls in case of windowWill/DidEnter/ExitFullScreen
  // and windowWill/DidStart/EndLiveResize notifications.
  int m_resizingCount = 0;

#if SK_SUPPORT_GPU
  sk_sp<const GrGLInterface> m_glInterfaces;
  NSOpenGLContext* m_nsGL;
  NSOpenGLPixelFormat* m_nsPixelFormat;
  sk_sp<GrContext> m_grCtx;
  sk_sp<SkSurface> m_skSurfaceDirect;
  sk_sp<SkSurface> m_skSurface;
#endif

  DISABLE_COPYING(SkiaWindowOSX);
};

} // namespace os

#endif
