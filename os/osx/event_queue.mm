// LAF OS Library
// Copyright (C) 2015-2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include "os/osx/event_queue.h"

#define EV_TRACE(...)

namespace os {

void OSXEventQueue::getEvent(Event& ev, bool canWait)
{
  ev.setType(Event::None);

  @autoreleasepool {
    NSApplication* app = [NSApplication sharedApplication];
    if (!app)
      return;

    NSEvent* event;
    do {
      // Pump the whole queue of Cocoa events
      event = [app nextEventMatchingMask:NSEventMaskAny
                               untilDate:[NSDate distantPast]
                                  inMode:NSDefaultRunLoopMode
                                 dequeue:YES];
    retry:
      if (event) {
        // Intercept <Control+Tab>, <Cmd+[>, and other keyboard
        // combinations, and send them directly to the main
        // NSView. Without this, the NSApplication intercepts the key
        // combination and use it to go to the next key view.
        if (event.type == NSEventTypeKeyDown) {
          [app.mainWindow.contentView keyDown:event];
        }
        else {
          [app sendEvent:event];
        }
      }
    } while (event);

    if (!m_events.try_pop(ev)) {
      if (canWait) {
        EV_TRACE("EV: Wait for events...\n");

        // Wait until there is a Cocoa event in queue
        event = [app nextEventMatchingMask:NSEventMaskAny
                                 untilDate:[NSDate distantFuture]
                                    inMode:NSDefaultRunLoopMode
                                   dequeue:YES];

        EV_TRACE("EV: Wake up!\n");
        goto retry;
      }
    }
  }
}

void OSXEventQueue::queueEvent(const Event& ev)
{
  m_events.push(ev);
}

} // namespace os
