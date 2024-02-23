// LAF Library
// Copyright (c) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "os/os.h"
#include "text/text.h"

#include <cstdio>

using namespace os;
using namespace text;

class MyDrawTextDelegate : public DrawTextDelegate {
  gfx::Point m_mousePos;
public:
  MyDrawTextDelegate(const gfx::Point& mousePos) : m_mousePos(mousePos) { }

  void preProcessChar(const int index,
                      const int codepoint,
                      gfx::Color& fg,
                      gfx::Color& bg,
                      const gfx::Rect& charBounds) override {
    if (charBounds.contains(m_mousePos)) {
      fg = gfx::rgba(0, 0, 0);
      bg = gfx::rgba(255, 255, 255);
    }
    else {
      fg = gfx::rgba(255, 255, 255);
      bg = gfx::rgba(0, 0, 0, 0);
    }
  }
};

void draw_window(Window* window,
                 const FontMgrRef& fontMgr,
                 const FontRef& font,
                 const gfx::Point& mousePos)
{
  Surface* surface = window->surface();
  SurfaceLock lock(surface);
  const gfx::Rect rc = surface->bounds();

  SurfaceRef backSurface = instance()->makeSurface(rc.w, rc.h);
  SurfaceLock lock2(backSurface.get());

  Paint p;
  p.color(gfx::rgba(0, 0, 0));
  p.style(Paint::Fill);
  backSurface->drawRect(rc, p);

  p.color(gfx::rgba(255, 255, 255));

  const char* lines[] = { "English",
                          "Español",
                          "Русский язык",   // Russian
                          "汉语",            // Simplified Chinese
                          "日本語",          // Japanese
                          "한국어",          // Korean
                          "العَرَبِيَّة‎",        // Arabic
                          "👍❤️😂☺️😯😢😡" }; // Emojis

  MyDrawTextDelegate delegate(mousePos);
  gfx::PointF pos(0, 0);
  for (auto line : lines) {
    std::string s = line;
    draw_text_with_shaper(
      backSurface.get(), fontMgr, font, s,
      pos, &p, TextAlign::Left, &delegate);

    pos.y += font->height() + 4;
  }

  // Flip the back surface to the window surface
  surface->drawSurface(backSurface.get(), 0, 0);

  // Invalidates the whole window to show it on the screen.
  if (window->isVisible())
    window->invalidateRegion(gfx::Region(rc));
  else
    window->setVisible(true);
}

int app_main(int argc, char* argv[])
{
  SystemRef system = make_system();
  system->setAppMode(AppMode::GUI);

  FontMgrRef fontMgr = FontMgr::Make();
  FontRef font = fontMgr->defaultFont(32);
  if (!font) {
    std::printf("Font not found\n");
    return 1;
  }

  WindowRef window = system->makeWindow(400, 300);
  window->setTitle("CTL");

  system->finishLaunching();
  system->activateApp();

  // Wait until a key is pressed or the window is closed
  EventQueue* queue = system->eventQueue();
  gfx::Point mousePos;
  bool running = true;
  bool redraw = true;
  while (running) {
    if (redraw) {
      redraw = false;
      draw_window(window.get(), fontMgr, font, mousePos);
    }

    Event ev;
    queue->getEvent(ev);
    switch (ev.type()) {

      case Event::CloseWindow:
        running = false;
        break;

      case Event::KeyDown:
        switch (ev.scancode()) {
          case kKeyEsc:
            running = false;
            break;
          case kKey1:
          case kKey2:
          case kKey3:
          case kKey4:
          case kKey5:
          case kKey6:
          case kKey7:
          case kKey8:
          case kKey9:
            // Set scale
            window->setScale(1 + (int)(ev.scancode() - kKey1));
            redraw = true;
            break;
          default:
            // Do nothing for other cases
            break;
        }
        break;

      case Event::ResizeWindow:
        redraw = true;
        break;

      case Event::MouseEnter:
      case Event::MouseMove:
        mousePos = ev.position();
        redraw = true;
        break;

      case Event::MouseLeave:
        mousePos = gfx::Point(-1, -1);
        redraw = true;
        break;

      default:
        // Do nothing
        break;
    }
  }

  return 0;
}
