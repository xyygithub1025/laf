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

const char* kTitle = "CTL";

class MyDrawTextDelegate : public DrawTextDelegate {
  gfx::Point m_mousePos;
  base::codepoint_t m_codepoint = 0;
public:
  MyDrawTextDelegate(const gfx::Point& mousePos) : m_mousePos(mousePos) { }

  // Codepoint of the char with the mouse above.
  base::codepoint_t codepoint() const { return m_codepoint; }

  void preProcessChar(const int index,
                      const base::codepoint_t codepoint,
                      gfx::Color& fg,
                      gfx::Color& bg,
                      const gfx::Rect& charBounds) override {
    if (charBounds.contains(m_mousePos)) {
      fg = gfx::rgba(0, 0, 0);
      bg = gfx::rgba(255, 255, 255);

      m_codepoint = codepoint;
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

  Paint p;
  p.color(gfx::rgba(0, 0, 0));
  p.style(Paint::Fill);
  surface->drawRect(rc, p);

  p.color(gfx::rgba(255, 255, 255));

  const char* lines[] = { "English",
                          "Español",
                          "Русский",        // Russian
                          "汉语",            // Simplified Chinese
                          "日本語",          // Japanese
                          "한국어",          // Korean
                          "العَرَبِيَّة‎",        // Arabic
                          "👍❤️😂☺️😯😢😡" }; // Emojis

  MyDrawTextDelegate delegate(mousePos);
  gfx::PointF pos(0, 0);
  for (auto line : lines) {
    std::string s = line;

#if 0
    draw_text_with_shaper(
      surface, fontMgr, font, s,
      pos, &p, TextAlign::Left);
#else
    // This example shows how to use the old DrawTextDelegate to
    // change foreground/background per character.
    draw_text(
      surface, fontMgr, font, s,
      p.color(), gfx::rgba(0, 0, 0),
      pos.x, pos.y, &delegate);
#endif

    pos.y += font->height() + 4;
  }

  // Show Unicode code point of the hover char in the title bar.
  if (delegate.codepoint()) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s - U+%04X", kTitle, delegate.codepoint());
    window->setTitle(buf);
  }
  else {
    window->setTitle(kTitle);
  }

  // Invalidates the whole window to show it on the screen.
  if (window->isVisible())
    window->invalidateRegion(gfx::Region(rc));
  else
    window->setVisible(true);
}

int app_main(int argc, char* argv[])
{
  SystemRef system = System::make();
  system->setAppMode(AppMode::GUI);

  FontMgrRef fontMgr = FontMgr::Make();
  FontRef font = fontMgr->defaultFont(32);
  if (!font) {
    std::printf("Font not found\n");
    return 1;
  }

  WindowRef window = system->makeWindow(400, 300);
  window->setTitle(kTitle);

  system->finishLaunching();
  system->activateApp();

  // Wait until a key is pressed or the window is closed
  EventQueue* queue = system->eventQueue();
  gfx::Point mousePos;
  bool running = true;
  bool redraw = true;

  system->handleWindowResize = [&](Window* w) {
    draw_window(w, fontMgr, font, mousePos);
  };

  while (running) {
    // Pick next event in the queue (without waiting)
    Event ev;
    queue->getEvent(ev, 0.0);

    // If there are no more events in the queue (Event::None type),
    // redraw the window and wait for some event. We do this so we
    // don't need to redraw the text on each mouse movement (we can
    // process several mouse movements at the same time and redraw
    // when the messages stop).
    if (ev.type() == Event::None) {
      if (redraw) {
        redraw = false;
        draw_window(window.get(), fontMgr, font, mousePos);
      }
      queue->getEvent(ev);
    }

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
