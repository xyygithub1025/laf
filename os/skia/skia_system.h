// LAF OS Library
// Copyright (C) 2018-2021  Igara Studio S.A.
// Copyright (C) 2012-2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_SKIA_SKIA_SYSTEM_INCLUDED
#define OS_SKIA_SKIA_SYSTEM_INCLUDED
#pragma once

#include "gfx/color_space.h"
#include "gfx/size.h"
#include "os/common/system.h"
#include "os/display_spec.h"
#include "os/skia/skia_color_space.h"
#include "os/skia/skia_display.h"
#include "os/skia/skia_font_manager.h"
#include "os/skia/skia_surface.h"

#ifdef _WIN32
  #include "os/win/color_space.h"
  #include "os/win/system.h"
  #define SkiaSystemBase WindowSystem
#elif __APPLE__
  #include "os/osx/color_space.h"
  #include "os/osx/system.h"
  #define SkiaSystemBase OSXSystem
#else
  #include "os/x11/system.h"
  #define SkiaSystemBase X11System
#endif

#include "SkGraphics.h"

#include <algorithm>
#include <memory>

namespace os {

class SkiaSystem : public SkiaSystemBase {
public:
  SkiaSystem()
    : m_defaultDisplay(nullptr)
    , m_gpuAcceleration(false) {
    SkGraphics::Init();
  }

  ~SkiaSystem() {
    SkGraphics::Term();
  }

  Capabilities capabilities() const override {
    return Capabilities(
      int(Capabilities::MultipleDisplays) |
      int(Capabilities::CanResizeDisplay) |
      int(Capabilities::DisplayScale) |
      int(Capabilities::CustomNativeMouseCursor) |
      int(Capabilities::ColorSpaces)
    // TODO enable this when the GPU support is ready
#if 0 // SK_SUPPORT_GPU
      | int(Capabilities::GpuAccelerationSwitch)
#endif
      );
  }

  bool gpuAcceleration() const override {
    return m_gpuAcceleration;
  }

  void setGpuAcceleration(bool state) override {
    m_gpuAcceleration = state;
  }

  void setTabletAPI(TabletAPI api) override {
    SkiaSystemBase::setTabletAPI(api);
    if (SkiaDisplay* display = dynamic_cast<SkiaDisplay*>(defaultDisplay())) {
      display->onTabletAPIChange();
    }
  }

  gfx::Size defaultNewDisplaySize() override {
    gfx::Size sz;
#ifdef _WIN32
    sz.w = GetSystemMetrics(SM_CXMAXIMIZED);
    sz.h = GetSystemMetrics(SM_CYMAXIMIZED);
    sz.w -= GetSystemMetrics(SM_CXSIZEFRAME)*4;
    sz.h -= GetSystemMetrics(SM_CYSIZEFRAME)*4;
    sz.w = std::max(0, sz.w);
    sz.h = std::max(0, sz.h);
#endif
    return sz;
  }

  Display* defaultDisplay() override {
    return m_defaultDisplay;
  }

  DisplayRef makeDisplay(const DisplaySpec& spec) override {
    auto display = make_ref<SkiaDisplay>(spec);
    if (!m_defaultDisplay)
      m_defaultDisplay = display.get();
    if (display && m_displayCS)
      display->setColorSpace(m_displayCS);
    return display;
  }

  SurfaceRef makeSurface(int width, int height,
                         const os::ColorSpaceRef& colorSpace) override {
    auto sur = make_ref<SkiaSurface>();
    sur->create(width, height, colorSpace);
    return sur;
  }

  SurfaceRef makeRgbaSurface(int width, int height,
                             const os::ColorSpaceRef& colorSpace) override {
    auto sur = make_ref<SkiaSurface>();
    sur->createRgba(width, height, colorSpace);
    return sur;
  }

  SurfaceRef loadSurface(const char* filename) override {
    return SkiaSurface::loadSurface(filename);
  }

  SurfaceRef loadRgbaSurface(const char* filename) override {
    return loadSurface(filename);
  }

  FontManager* fontManager() override {
    if (!m_fontManager)
      m_fontManager.reset(new SkiaFontManager);
    return m_fontManager.get();
  }

  void setTranslateDeadKeys(bool state) override {
    if (m_defaultDisplay)
      m_defaultDisplay->setTranslateDeadKeys(state);
  }

  void listColorSpaces(std::vector<os::ColorSpaceRef>& list) override {
    list.push_back(makeColorSpace(gfx::ColorSpace::MakeNone()));
    list.push_back(makeColorSpace(gfx::ColorSpace::MakeSRGB()));

#if defined(_WIN32) || defined(__APPLE__)
    list_display_colorspaces(list);
#endif
  }

  os::ColorSpaceRef makeColorSpace(const gfx::ColorSpaceRef& cs) override {
    return os::make_ref<SkiaColorSpace>(cs);
  }

  Ref<ColorSpaceConversion> convertBetweenColorSpace(
    const os::ColorSpaceRef& src,
    const os::ColorSpaceRef& dst) override {
    return os::make_ref<SkiaColorSpaceConversion>(src, dst);
  }

  void setDisplaysColorSpace(const os::ColorSpaceRef& cs) override {
    m_displayCS = cs;
    if (m_defaultDisplay) {
      m_defaultDisplay->setColorSpace(
        m_displayCS ? m_displayCS:
                      m_defaultDisplay->currentMonitorColorSpace());
    }
  }

  os::ColorSpaceRef displaysColorSpace() override {
    return m_displayCS;
  }

private:
  SkiaDisplay* m_defaultDisplay;
  Ref<FontManager> m_fontManager;
  bool m_gpuAcceleration;
  ColorSpaceRef m_displayCS;
};

} // namespace os

#endif
