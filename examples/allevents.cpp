// LAF Library
// Copyright (c) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "gfx/hsv.h"
#include "gfx/rgb.h"
#include "os/os.h"
#include "text/text.h"

#include <algorithm>
#include <cstdarg>
#include <cstdlib>
#include <string>
#include <vector>

using namespace os;
using namespace text;

class LogWindow {
public:
  LogWindow(System* system)
    : m_window(system->makeWindow(800, 600))
    , m_font(FontMgr::Make()->defaultFont(12)) {
    m_window->setTitle("All Events");

    recalcMaxLines();

    logLine("-- Events Log --");
  }

  bool processEvent(const Event& ev) {
    switch (ev.type()) {

      case Event::CloseApp:
      case Event::CloseWindow:
        return false;

      case Event::ResizeWindow:
        logLine("ResizeWindow size=%d,%d",
                m_window->width(),
                m_window->height());
        recalcMaxLines();
        break;

      case Event::DropFiles:
        logLine("DropFiles files={");
        for (const auto& file : ev.files()) {
          logLine("  \"%s\"", file.c_str());
        }
        logLine("}");
        break;

      case Event::MouseEnter: logMouseEvent(ev, "MouseEnter"); break;
      case Event::MouseLeave: logMouseEvent(ev, "MouseLeave"); break;
      case Event::MouseMove: logMouseEvent(ev, "MouseMove"); break;
      case Event::MouseDown: logMouseEvent(ev, "MouseDown"); break;
      case Event::MouseUp: logMouseEvent(ev, "MouseUp"); break;
      case Event::MouseDoubleClick: logMouseEvent(ev, "MouseDoubleClick"); break;

      case Event::MouseWheel:
        m_mousePos = ev.position();
        logLine("MouseWheel pos=%d,%d %s=%d,%d%s",
                ev.position().x,
                ev.position().y,
                ev.preciseWheel() ? " preciseWheel": "wheel",
                ev.wheelDelta().x,
                ev.wheelDelta().y,
                modifiersToString(ev.modifiers()).c_str());
        m_hue += double(ev.wheelDelta().x + ev.wheelDelta().y);
        break;

      case Event::KeyDown:
        if (ev.scancode() == kKeyEsc) {
          if (m_nextEscCloses)
            return false;
          else
            m_nextEscCloses = true;
          logLine("-- Next KeyDown with kKeyEsc will close the window --");
        }
        else {
          m_nextEscCloses = false;
        }
        //[[fallthrough]];
      case Event::KeyUp: {
        wchar_t wideUnicode[2] = { ev.unicodeChar(), 0 };
        logLine("%s repeat=%d scancode=%d unicode=%d (%s)%s",
                (ev.type() == Event::KeyDown ? "KeyDown": "KeyUp"),
                ev.repeat(),
                ev.scancode(),
                ev.unicodeChar(),
                base::to_utf8(wideUnicode).c_str(),
                modifiersToString(ev.modifiers()).c_str());
        break;
      }

      case Event::TouchMagnify:
        logLine("TouchMagnify %.4g",
                ev.magnification());
        m_brushSize += 32*ev.magnification();
        m_brushSize = std::clamp(m_brushSize, 1.0, 500.0);
        break;

      default:
        // Do nothing
        break;
    }
    return true;
  }

  void flush() {
    if (m_oldLogSize != m_textLog.size()) {
      int newlines = m_textLog.size() - m_oldLogSize;
      while (m_textLog.size() > m_maxlines)
        m_textLog.erase(m_textLog.begin());

      scrollAndDrawLog(newlines);

      m_oldLogSize = m_textLog.size();
    }
  }

private:
  void recalcMaxLines() {
    m_maxlines = (m_window->height() - m_lineHeight) / m_lineHeight;
  }

  void scrollAndDrawLog(const int newlines) {
    Surface* surface = m_window->surface();
    SurfaceLock lock(surface);
    const gfx::Rect rc = surface->bounds();

    Paint p;
    p.style(Paint::Fill);
    p.color(gfx::rgba(0, 0, 0, 8));

    // Scroll old lines
    int i;
    if (m_textLog.size() >= m_maxlines) {
      int h = m_lineHeight*newlines;
      surface->scrollTo(rc, 0, -h);

      surface->drawRect(gfx::Rect(rc.x, rc.y, rc.w, rc.h-h), p);
      p.color(gfx::rgba(0, 0, 0));
      surface->drawRect(gfx::Rect(rc.x, rc.y+rc.h-h, rc.w, h), p);

      i = (m_textLog.size()-newlines);
    }
    // First lines without scroll
    else {
      i = m_oldLogSize;
      surface->drawRect(gfx::Rect(rc.x, rc.y, rc.w, i*m_lineHeight), p);
    }

    Paint paint;
    paint.color(gfx::rgba(255, 255, 255));
    for (; i<m_textLog.size(); ++i)
      draw_text(surface, m_font, m_textLog[i],
                gfx::Point(0, (1+i)*m_lineHeight), &paint);

    gfx::Rgb rgb(gfx::Hsv(m_hue, 1.0, 1.0));
    paint.color(gfx::rgba(rgb.red(), rgb.green(), rgb.blue()));
    paint.antialias(true);
    surface->drawCircle(m_mousePos.x, m_mousePos.y, m_brushSize, paint);

    // Invalidates the whole window to show it on the screen.
    if (m_window->isVisible())
      m_window->invalidateRegion(gfx::Region(rc));
    else
      m_window->setVisible(true);
  }

  void logMouseEvent(const Event& ev, const char* eventName) {
    const Event::MouseButton mb = ev.button();
    const PointerType pt = ev.pointerType();

    m_mousePos = ev.position();
    logLine("%s pos=%d,%d%s%s%s",
            eventName,
            ev.position().x,
            ev.position().y,
            (mb == Event::LeftButton ? " LeftButton":
             mb == Event::RightButton ? " RightButton":
             mb == Event::MiddleButton ? " MiddleButton":
             mb == Event::X1Button ? " X1Button":
             mb == Event::X2Button ? " X2Button": ""),
            (pt == PointerType::Mouse ? " Mouse":
             pt == PointerType::Touchpad ? " Touchpad":
             pt == PointerType::Touch ? " Touch":
             pt == PointerType::Pen ? " Pen":
             pt == PointerType::Cursor ? " Cursor":
             pt == PointerType::Eraser ? " Eraser": ""),
            modifiersToString(ev.modifiers()).c_str());
  }

  void logLine(const char* str, ...) {
    va_list ap;
    va_start(ap, str);
    char buf[4096];
    vsprintf(buf, str, ap);
    va_end(ap);

    m_textLog.push_back(buf);
  }

  static std::string modifiersToString(KeyModifiers mods) {
    std::string s;
    if (mods & kKeyShiftModifier) s += " Shift";
    if (mods & kKeyCtrlModifier ) s += " Ctrl";
    if (mods & kKeyAltModifier  ) s += " Alt";
    if (mods & kKeyCmdModifier  ) s += " Command";
    if (mods & kKeySpaceModifier) s += " Space";
    if (mods & kKeyWinModifier  ) s += " Win";
    return s;
  }

  WindowRef m_window;
  FontRef m_font;
  std::vector<std::string> m_textLog;
  size_t m_oldLogSize = 0;
  int m_lineHeight = 12;
  int m_maxlines = 0;
  gfx::Point m_mousePos;
  double m_brushSize = 4;
  double m_hue = 0.0;
  bool m_nextEscCloses = false;
};

int app_main(int argc, char* argv[])
{
  auto system = make_system();
  system->setAppMode(AppMode::GUI);

  LogWindow window(system.get());

  system->finishLaunching();
  system->activateApp();

  EventQueue* queue = system->eventQueue();
  while (true) {
    window.flush();

    Event ev;
    queue->getEvent(ev);
    if (!window.processEvent(ev))
      break;
  }

  return 0;
}
