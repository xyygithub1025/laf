// LAF OS Library
// Copyright (C) 2019-2024  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_COMMON_SYSTEM_H
#define OS_COMMON_SYSTEM_H
#pragma once

#include "os/event_queue.h"
#include "os/menus.h"
#include "os/system.h"

namespace os {

class CommonSystem : public System {
public:
  CommonSystem();
  ~CommonSystem();

  void setAppName(const std::string& appName) override { }
  void setAppMode(AppMode appMode) override { }

  void markCliFileAsProcessed(const std::string& fn) override { }
  void finishLaunching() override { }
  void activateApp() override { }

  Capabilities capabilities() const override {
    return (Capabilities)0;
  }

  void setTabletAPI(TabletAPI api) override {
    // Do nothing by default
  }

  TabletAPI tabletAPI() const override {
    return TabletAPI::Default;
  }

  void errorMessage(const char* msg) override;

  Logger* logger() override {
    return nullptr;
  }

  Menus* menus() override {
    return nullptr;
  }

  NativeDialogs* nativeDialogs() override;

  EventQueue* eventQueue() override {
    return EventQueue::instance();
  }

  KeyModifiers keyModifiers() override;
  ScreenRef mainScreen() override { return nullptr; }
  void listScreens(ScreenList& screens) override { }
  Window* defaultWindow() override { return nullptr; }
  Ref<Window> makeWindow(const WindowSpec&) override { return nullptr; }
  Ref<Surface> makeSurface(int, int, const os::ColorSpaceRef&) override { return nullptr; }
#if CLIP_ENABLE_IMAGE
  Ref<Surface> makeSurface(const clip::image& image) override;
#endif
  Ref<Surface> makeRgbaSurface(int, int, const os::ColorSpaceRef&) override { return nullptr; }
  Ref<Surface> loadSurface(const char*) override { return nullptr; }
  Ref<Surface> loadRgbaSurface(const char*) override { return nullptr; }
  Ref<Cursor> makeCursor(const Surface*, const gfx::Point&, int) override { return nullptr; }
  bool isKeyPressed(KeyScancode) override { return false; }
  int getUnicodeFromScancode(KeyScancode) override { return 0; }
  void setTranslateDeadKeys(bool) override { }
  gfx::Point mousePosition() const override { return gfx::Point(0, 0); }
  void setMousePosition(const gfx::Point&) override { }
  gfx::Color getColorFromScreen(const gfx::Point&) const override { return gfx::ColorNone; }
  void listColorSpaces(std::vector<os::ColorSpaceRef>&) override { }
  os::ColorSpaceRef makeColorSpace(const gfx::ColorSpaceRef&) override { return nullptr; }
  Ref<ColorSpaceConversion> convertBetweenColorSpace(
    const os::ColorSpaceRef&, const os::ColorSpaceRef&) override {
      return nullptr;
  }
  void setWindowsColorSpace(const os::ColorSpaceRef&) override { }
  os::ColorSpaceRef windowsColorSpace() override { return nullptr; }

protected:
  void destroyInstance();

private:
  Ref<NativeDialogs> m_nativeDialogs;
};

} // namespace os

#endif
