// LAF OS Library
// Copyright (C) 2019-2021  Igara Studio S.A.
// Copyright (C) 2016-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/x11/event_queue.h"

#include "os/x11/window.h"

#include <X11/Xlib.h>

#include <sys/select.h>

#define EV_TRACE(...)

namespace os {

namespace {

#if !defined(NDEBUG)
const char* get_event_name(XEvent& event)
{
  switch (event.type) {
    case KeyPress: return "KeyPress";
    case KeyRelease: return "KeyRelease";
    case ButtonPress: return "ButtonPress";
    case ButtonRelease: return "ButtonRelease";
    case MotionNotify: return "MotionNotify";
    case EnterNotify: return "EnterNotify";
    case LeaveNotify: return "LeaveNotify";
    case FocusIn: return "FocusIn";
    case FocusOut: return "FocusOut";
    case KeymapNotify: return "KeymapNotify";
    case Expose: return "Expose";
    case GraphicsExpose: return "GraphicsExpose";
    case NoExpose: return "NoExpose";
    case VisibilityNotify: return "VisibilityNotify";
    case CreateNotify: return "CreateNotify";
    case DestroyNotify: return "DestroyNotify";
    case UnmapNotify: return "UnmapNotify";
    case MapNotify: return "MapNotify";
    case MapRequest: return "MapRequest";
    case ReparentNotify: return "ReparentNotify";
    case ConfigureNotify: return "ConfigureNotify";
    case ConfigureRequest: return "ConfigureRequest";
    case GravityNotify: return "GravityNotify";
    case ResizeRequest: return "ResizeRequest";
    case CirculateNotify: return "CirculateNotify";
    case CirculateRequest: return "CirculateRequest";
    case PropertyNotify: return "PropertyNotify";
    case SelectionClear: return "SelectionClear";
    case SelectionRequest: return "SelectionRequest";
    case SelectionNotify: return "SelectionNotify";
    case ColormapNotify: return "ColormapNotify";
    case ClientMessage: return "ClientMessage";
    case MappingNotify: return "MappingNotify";
    case GenericEvent: return "GenericEvent";
  }
  return "Unknown";
}
#endif

void wait_file_descriptor_for_reading(int fd, base::tick_t timeoutMilliseconds)
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);

  timeval timeout;
  timeout.tv_sec = timeoutMilliseconds / 1000;
  timeout.tv_usec = ((timeoutMilliseconds % 1000) * 1000);

  // First argument must be set to the highest-numbered file
  // descriptor in any of the three sets, plus 1.
  select(fd+1, &fds, nullptr, nullptr, &timeout);
}

} // anonymous namespace

void EventQueueX11::queueEvent(const Event& ev)
{
  m_events.push(ev);
}

void EventQueueX11::getEvent(Event& ev, double timeout)
{
  base::tick_t startTime = base::current_tick();

  ev.setWindow(nullptr);

  ::Display* display = X11::instance()->display();
  XSync(display, False);

  XEvent event;
  int events = XEventsQueued(display, QueuedAlready);
  if (events == 0) {
    if (timeout == kWithoutTimeout) {
      events = 1;
    }
    else if (timeout > 0.0) {
      // Wait timeout (waiting the X11 connection file description for
      // a read operation). We've to use this method to wait for
      // events with timeout because we don't have a X11 function like
      // XNextEvent() with a timeout.
      base::tick_t timeoutMsecs = base::tick_t(timeout * 1000.0);
      base::tick_t elapsedMsecs = base::current_tick() - startTime;
      if (timeoutMsecs - elapsedMsecs > 0) {
        int connFileDesc = ConnectionNumber(display);
        wait_file_descriptor_for_reading(connFileDesc,
                                         timeoutMsecs - elapsedMsecs);
      }

      events = XEventsQueued(display, QueuedAlready);
    }
  }

  for (int i=0; i<events; ++i) {
    XNextEvent(display, &event);
    processX11Event(event);
  }

  if (!m_events.try_pop(ev)) {
#pragma push_macro("None")
#undef None // Undefine the X11 None macro
    ev.setType(Event::None);
#pragma pop_macro("None")
  }
}

void EventQueueX11::clearEvents()
{
  m_events.clear();
}

void EventQueueX11::processX11Event(XEvent& event)
{
  EV_TRACE("XEvent: %s (%d)\n", get_event_name(event), event.type);

  WindowX11* window = WindowX11::getPointerFromHandle(event.xany.window);
  // In MappingNotify the window can be nullptr
  if (window)
    window->processX11Event(event);
}

} // namespace os
