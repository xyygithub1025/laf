// LAF OS Library
// Copyright (C) 2021  Igara Studio S.A.
// Copyright (C) 2016  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/x11/x11.h"

#include "base/debug.h"
#include "os/x11/event_queue.h"

namespace os {

X11* X11::m_instance = nullptr;

// static
X11* X11::instance()
{
  ASSERT(m_instance);
  return m_instance;
}

X11::X11()
{
  ASSERT(m_instance == nullptr);
  m_instance = this;

  // TODO We shouldn't need to call this function (because we
  // shouldn't be using the m_display from different threads), but
  // it might be necessary?
  // https://github.com/aseprite/aseprite/issues/1962
  XInitThreads();

  m_display = XOpenDisplay(nullptr);
  m_xim = XOpenIM(m_display, nullptr, nullptr, nullptr);
  if (m_display)
    m_xinput.load(m_display);
}

X11::~X11()
{
  ASSERT(m_instance == this);

  // TODO This is bad, really bad. We have to clear the list of events
  //      to clear all references to displays and delete them here
  //      (before closing the X11 display connection).
  //
  //      The event queue should be inside the System instance (so
  //      when the system is deleted, the queue is deleted).
  ((os::X11EventQueue*)os::EventQueue::instance())->clear();

  if (m_xim) {
    XCloseIM(m_xim);
  }
  if (m_display) {
    m_xinput.unload(m_display);
    XCloseDisplay(m_display);
  }
  m_instance = nullptr;
}

} // namespace os
