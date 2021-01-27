// LAF Library
// Copyright (c) 2019-2021  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "base/clamp.h"
#include "gfx/hsv.h"
#include "gfx/rgb.h"
#include "os/os.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <vector>

static std::vector<os::DisplayRef> displays;

const char* lines[] = { "A: Switch mouse cursor to Arrow <-> Move",
                        "H: Hide window (or show all windows again)",
                        "",
                        "C: Change display frame to content",
                        "F: Change display content to frame",
                        "W: Change display content to workarea",
                        "",
                        "D: Duplicate display",
                        "",
                        "Q: Close all windows",
                        "ESC: Close this display" };

static void redraw_display(os::Display* display)
{
  os::Surface* s = display->surface();
  os::Paint paint;
  paint.color(gfx::rgba(0, 0, 0));
  s->drawRect(display->bounds(), paint);

  paint.color(gfx::rgba(255, 255, 255));

  char buf[256];
  int y = 12;

  gfx::Rect rc = display->frame();
  std::sprintf(buf, "Frame = (%d %d %d %d)", rc.x, rc.y, rc.w, rc.h);
  os::draw_text(s, nullptr, buf, gfx::Point(0, y), &paint);
  y += 12;

  rc = display->contentRect();
  std::sprintf(buf, "Content Rect = (%d %d %d %d)", rc.x, rc.y, rc.w, rc.h);
  os::draw_text(s, nullptr, buf, gfx::Point(0, y), &paint);
  y += 12;

  for (auto line : lines) {
    y += 12;
    os::draw_text(s, nullptr, line, gfx::Point(0, y), &paint);
  }

  paint.style(os::Paint::Style::Stroke);
  s->drawRect(display->bounds(), paint);
}

static os::DisplayRef add_display(const std::string& title,
                                  const os::DisplaySpec& spec)
{
  os::DisplayRef newDisplay = os::instance()->makeDisplay(spec);
  newDisplay->setNativeMouseCursor(os::kArrowCursor);
  newDisplay->setTitle(title);
  newDisplay->handleResize = redraw_display;
  displays.emplace_back(newDisplay);

  redraw_display(newDisplay.get());
  return newDisplay;
}

static void check_show_all_displays()
{
  // If all displays are hidden, show then again
  auto hidden = std::count_if(displays.begin(), displays.end(),
                              [](os::DisplayRef display){
                                return !display->isVisible();
                              });
  if (hidden == displays.size()) {
    std::for_each(displays.begin(), displays.end(),
                  [](os::DisplayRef display){
                    display->setVisible(true);
                  });
  }
}

static void destroy_display(const os::DisplayRef& display)
{
  auto it = std::find(displays.begin(), displays.end(), display);
  if (it != displays.end())
    displays.erase(it);

  check_show_all_displays();
}

int app_main(int argc, char* argv[])
{
  auto system = os::make_system();
  system->setAppMode(os::AppMode::GUI);

  // Create four windows for each screen with the bounds of the
  // workarea.
  os::ScreenList screens;
  system->listScreens(screens);
  char chr = 'A';
  for (os::ScreenRef& screen : screens) {
    os::DisplaySpec spec;
    spec.titled(true);
    spec.position(os::DisplaySpec::Position::Frame);
    spec.frame(screen->workarea());
    spec.screen(screen);

    gfx::PointF pos[4] = { gfx::PointF(0.0, 0.0),
                           gfx::PointF(0.5, 0.0),
                           gfx::PointF(0.0, 0.5),
                           gfx::PointF(0.5, 0.5) };
    for (auto& p : pos) {
      os::DisplaySpec s = spec;
      gfx::Rect frame = s.frame();
      frame.x += frame.w*p.x;
      frame.y += frame.h*p.y;
      frame.w /= 2;
      frame.h /= 2;
      s.frame(frame);
      add_display(std::string(1, chr++), s);
    }
  }

  system->finishLaunching();
  system->activateApp();

  os::EventQueue* queue = system->eventQueue();
  os::Event ev;
  while (!displays.empty()) {
    queue->getEvent(ev, true);

    switch (ev.type()) {

      case os::Event::CloseApp:
        displays.clear(); // Close all displays
        break;

      case os::Event::CloseDisplay:
        destroy_display(ev.display());
        break;

      case os::Event::ResizeDisplay:
        redraw_display(ev.display().get());
        ev.display()->invalidate();
        break;

      case os::Event::KeyDown:
        switch (ev.scancode()) {

          case os::kKeyQ:
            displays.clear();
            break;

          case os::kKeyEsc:
            destroy_display(ev.display());
            break;

          // Switch between Arrow/Move cursor in this specific display
          case os::kKeyA:
            ev.display()->setNativeMouseCursor(
              ev.display()->nativeMouseCursor() == os::kArrowCursor ?
                os::kMoveCursor:
                os::kArrowCursor);
            break;

          case os::kKeyH:
            ev.display()->setVisible(!ev.display()->isVisible());
            check_show_all_displays();
            break;

          // Duplicate display
          case os::kKeyD: {
            std::string title = ev.display()->title();
            os::DisplaySpec spec;
            spec.position(os::DisplaySpec::Position::Frame);
            spec.frame(ev.display()->frame());
            add_display(title, spec);
            break;
          }

          case os::kKeyF:
          case os::kKeyC:
          case os::kKeyW: {
            std::string title = ev.display()->title();
            os::DisplaySpec spec;
            if (ev.scancode() == os::kKeyF) {
              spec.position(os::DisplaySpec::Position::ContentRect);
              spec.contentRect(ev.display()->frame());
            }
            else if (ev.scancode() == os::kKeyC) {
              spec.position(os::DisplaySpec::Position::Frame);
              spec.frame(ev.display()->contentRect());
            }
            else if (ev.scancode() == os::kKeyW) {
              spec.position(os::DisplaySpec::Position::Frame);
              spec.frame(ev.display()->screen()->workarea());
            }

            // TODO add a new os::Display::setSpec() method instead of re-creating display
            destroy_display(ev.display());
            add_display(title, spec);
            break;
          }

          case os::kKeyLeft:
          case os::kKeyUp:
          case os::kKeyRight:
          case os::kKeyDown: {
            std::string title = ev.display()->title();
            os::DisplaySpec spec;
            gfx::Rect rc = ev.display()->frame();

            switch (ev.scancode()) {
              case os::kKeyLeft:  rc.x -= rc.w; break;
              case os::kKeyUp:    rc.y -= rc.h; break;
              case os::kKeyRight: rc.x += rc.w; break;
              case os::kKeyDown:  rc.y += rc.h; break;
            }

            spec.position(os::DisplaySpec::Position::Frame);
            spec.frame(rc);

            destroy_display(ev.display());
            add_display(title, spec);
            break;
          }

        }
        break;

      default:
        // Do nothing
        break;
    }
  }

  return 0;
}
