// LAF OS Library
// Copyright (C) 2020-2021  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_WIN_EVENT_QUEUE_INCLUDED
#define OS_WIN_EVENT_QUEUE_INCLUDED
#pragma once

#include "os/event.h"
#include "os/event_queue.h"

#include <queue>

namespace os {

class EventQueueWin : public EventQueue {
public:
  void queueEvent(const Event& ev) override;
  void getEvent(Event& ev, bool canWait) override;

private:
  std::queue<Event> m_events;
};

using EventQueueImpl = EventQueueWin;

} // namespace os

#endif
