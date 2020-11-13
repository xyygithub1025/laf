// LAF OS Library
// Copyright (c) 2018-2020  Igara Studio S.A.
// Copyright (c) 2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/memory.h"
#include "base/string.h"
#include "gfx/size.h"
#include "os/display.h"
#include "os/font.h"
#include "os/system.h"

namespace os {

class NoneSystem : public System {
public:
  void setAppName(const std::string& appName) override { }
  void setAppMode(AppMode appMode) override { }

  void markCliFileAsProcessed(const std::string& fn) override { }
  void finishLaunching() override { }
  void activateApp() override { }

  Capabilities capabilities() const override { return (Capabilities)0; }
  void setTabletAPI(TabletAPI api) override { }
  TabletAPI tabletAPI() const override { return TabletAPI::Default; }
  Logger* logger() override { return nullptr; }
  Menus* menus() override { return nullptr; }
  NativeDialogs* nativeDialogs() override { return nullptr; }
  EventQueue* eventQueue() override { return nullptr; }
  bool gpuAcceleration() const override { return false; }
  void setGpuAcceleration(bool state) override { }
  ScreenRef mainScreen() override { return nullptr; }
  void listScreens(ScreenList& screens) override { }
  gfx::Size defaultNewDisplaySize() override { return gfx::Size(0, 0); }
  Display* defaultDisplay() override { return nullptr; }
  Ref<Display> makeDisplay(int width, int height, int scale) override { return nullptr; }
  Ref<Surface> makeSurface(int width, int height,
                           const os::ColorSpaceRef& colorSpace) override { return nullptr; }
  Ref<Surface> makeRgbaSurface(int width, int height,
                               const os::ColorSpaceRef& colorSpace) override { return nullptr; }
  Ref<Surface> loadSurface(const char* filename) override { return nullptr; }
  Ref<Surface> loadRgbaSurface(const char* filename) override { return nullptr; }
  FontManager* fontManager() override { return nullptr; }
  Ref<Font> loadSpriteSheetFont(const char* filename, int scale) override { return nullptr; }
  Ref<Font> loadTrueTypeFont(const char* filename, int height) override { return nullptr; }
  bool isKeyPressed(KeyScancode scancode) override { return false; }
  KeyModifiers keyModifiers() override { return kKeyNoneModifier; }
  int getUnicodeFromScancode(KeyScancode scancode) override { return 0; }
  void setTranslateDeadKeys(bool state) override { }
  void listColorSpaces(std::vector<os::ColorSpaceRef>& list) override { }
  os::ColorSpaceRef makeColorSpace(const gfx::ColorSpaceRef& cs) override { return nullptr; }
  Ref<ColorSpaceConversion> convertBetweenColorSpace(
    const os::ColorSpaceRef& src, const os::ColorSpaceRef& dst) override { return nullptr; }
  void setDisplaysColorSpace(const os::ColorSpaceRef& cs) override { }
  os::ColorSpaceRef displaysColorSpace() override { return nullptr; }
};

System* make_system_impl() {
  return new NoneSystem;
}

void error_message(const char* msg)
{
  fputs(msg, stderr);
  // TODO
}

} // namespace os
