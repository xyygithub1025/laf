// LAF OS Library
// Copyright (C) 2019-2023  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_COMMON_SYSTEM_H
#define OS_COMMON_SYSTEM_H
#pragma once

#if LAF_WINDOWS
  #include "os/win/native_dialogs.h"
#elif LAF_MACOS
  #include "os/osx/app.h"
  #include "os/osx/menus.h"
  #include "os/osx/native_dialogs.h"
#elif LAF_LINUX
  #include "os/x11/native_dialogs.h"
#else
  #include "os/native_dialogs.h"
#endif

#include "os/event_queue.h"
#include "os/menus.h"
#include "os/system.h"

namespace os {

class CommonSystem : public System {
public:
  CommonSystem() { }
  ~CommonSystem() {
    destroyInstance();
  }

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

  NativeDialogs* nativeDialogs() override {
    if (!m_nativeDialogs) {
#if LAF_WINDOWS
      m_nativeDialogs = Ref<NativeDialogs>(new NativeDialogsWin);
#elif LAF_MACOS
      m_nativeDialogs = Ref<NativeDialogs>(new NativeDialogsOSX);
#elif LAF_LINUX
      m_nativeDialogs = Ref<NativeDialogs>(new NativeDialogsX11);
#endif
    }
    return m_nativeDialogs.get();
  }

  EventQueue* eventQueue() override {
    return EventQueue::instance();
  }

  KeyModifiers keyModifiers() override {
    return
      (KeyModifiers)
      ((isKeyPressed(kKeyLShift) ||
        isKeyPressed(kKeyRShift) ? kKeyShiftModifier: 0) |
       (isKeyPressed(kKeyLControl) ||
        isKeyPressed(kKeyRControl) ? kKeyCtrlModifier: 0) |
       (isKeyPressed(kKeyAlt) ? kKeyAltModifier: 0) |
       (isKeyPressed(kKeyAltGr) ? (kKeyCtrlModifier | kKeyAltModifier): 0) |
       (isKeyPressed(kKeyCommand) ? kKeyCmdModifier: 0) |
       (isKeyPressed(kKeySpace) ? kKeySpaceModifier: 0) |
       (isKeyPressed(kKeyLWin) ||
        isKeyPressed(kKeyRWin) ? kKeyWinModifier: 0));
  }

  bool gpuAcceleration() const override { return false; }
  void setGpuAcceleration(bool state) override { }
  ScreenRef mainScreen() override { return nullptr; }
  void listScreens(ScreenList& screens) override { }
  Window* defaultWindow() override { return nullptr; }
  Ref<Window> makeWindow(const WindowSpec&) override { return nullptr; }
  Ref<Surface> makeSurface(int, int, const os::ColorSpaceRef&) override { return nullptr; }
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
