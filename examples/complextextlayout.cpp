// LAF Library
// Copyright (c) 2019  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "os/os.h"

void draw_display(os::Display* display)
{
  os::Surface* surface = display->getSurface();
  os::SurfaceLock lock(surface);
  const gfx::Rect rc = surface->bounds();

  os::Paint p;
  p.color(gfx::rgba(0, 0, 0));
  p.style(os::Paint::Fill);
  surface->drawRect(rc, p);

  p.color(gfx::rgba(255, 255, 255));

  const wchar_t* lines[] = { L"English",
                             L"Русский язык", // Russian
                             L"汉语",         // Simplified Chinese
                             L"日本語",       // Japanese
                             L"한국어",       // Korean
                             L"العَرَبِيَّة‎"  };     // Arabic

  gfx::Point pos(0, 0);
  for (auto line : lines) {
    os::draw_text_with_shaper(
      surface, nullptr, base::to_utf8(line), pos, &p);
    pos.x += 0;
    pos.y += 32;
  }

  // Invalidates the whole display to show it on the screen.
  display->invalidateRegion(gfx::Region(rc));
}

int app_main(int argc, char* argv[])
{
  os::ScopedHandle<os::System> system(os::create_system());
  os::ScopedHandle<os::Display> display(system->createDisplay(400, 300, 1));

  display->setNativeMouseCursor(os::kArrowCursor);
  display->setTitle("CTL");

  system->finishLaunching();
  system->activateApp();

  // Wait until a key is pressed or the window is closed
  os::EventQueue* queue = system->eventQueue();
  bool running = true;
  bool redraw = true;
  while (running) {
    if (redraw) {
      redraw = false;
      draw_display(display);
    }
    // Wait for an event in the queue, the "true" parameter indicates
    // that we'll wait for a new event, and the next line will not be
    // processed until we receive a new event. If we use "false" and
    // there is no events in the queue, we receive an "ev.type() == Event::None
    os::Event ev;
    queue->getEvent(ev, true);

    switch (ev.type()) {

      case os::Event::CloseDisplay:
        running = false;
        break;

      case os::Event::KeyDown:
        switch (ev.scancode()) {
          case os::kKeyEsc:
            running = false;
            break;
          case os::kKey1:
          case os::kKey2:
          case os::kKey3:
          case os::kKey4:
          case os::kKey5:
          case os::kKey6:
          case os::kKey7:
          case os::kKey8:
          case os::kKey9:
            // Set scale
            display->setScale(1 + (int)(ev.scancode() - os::kKey1));
            redraw = true;
            break;
          default:
            // Do nothing for other cases
            break;
        }
        break;

      case os::Event::ResizeDisplay:
        redraw = true;
        break;

      default:
        // Do nothing
        break;
    }
  }

  return 0;
}
