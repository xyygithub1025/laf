// LAF OS Library
// Copyright (C) 2021  Igara Studio S.A.
// Copyright (C) 2016-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_X11_EVENT_QUEUE_INCLUDED
#define OS_X11_EVENT_QUEUE_INCLUDED
#pragma once

#include "os/event_queue.h"
#include "os/x11/x11.h"

#include <queue>

namespace os {

class X11EventQueue : public EventQueue {
public:
  void queueEvent(const Event& ev) override;
  void getEvent(Event& ev, bool canWait) override;

private:
  void processX11Event(XEvent& event);

  std::queue<Event> m_events;
};

typedef X11EventQueue EventQueueImpl;

} // namespace os

#endif
