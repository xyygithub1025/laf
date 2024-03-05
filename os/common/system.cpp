// LAF OS Library
// Copyright (C) 2019-2024  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/common/system.h"

namespace os {

// Weak reference to the unique system instance. This is destroyed by
// the user (with the main SystemRef to the system).
System* g_instance = nullptr;

SystemRef System::instance()
{
  if (g_instance)
    return AddRef(g_instance);
  return nullptr;
}

SystemRef System::make()
{
  SystemRef ref;
#if LAF_SKIA
  ref = System::makeSkia();
#endif
#if LAF_WINDOWS
  if (!ref)
    ref = System::makeWin();
#elif LAF_MACOS
  if (!ref)
    ref = System::makeOSX();
#elif LAF_LINUX
  if (!ref)
    ref = System::makeX11();
#endif
  if (!ref)
    ref = System::makeNone();

  if (!g_instance)
    g_instance = ref.get();

  return ref;
}

void CommonSystem::errorMessage(const char* msg)
{
  fputs(msg, stderr);
}

// This must be called in the final class that derived from
// CommonSystem, because clearing the list of events can generate
// events on windows that will depend on the platform-specific
// System.
//
// E.g. We've crash reports because WindowWin is receiving
// WM_ACTIVATE messages when we destroy the events queue, and the
// handler of that message is expecting the SystemWin instance (not
// a CommonSystem instance). That's why we cannot call this from
// ~CommonSystem() destructor and we have to call this from
// ~SystemWin (or other platform-specific System implementations).
void CommonSystem::destroyInstance()
{
  // destroyInstance() can be called multiple times by derived
  // classes.
  if (g_instance != this)
    return;

  // We have to reset the list of all events to clear all possible
  // living WindowRef (so all window destructors are called at this
  // point, when the os::System instance is still alive).
  //
  // TODO Maybe the event queue should be inside the System instance
  //      (so when the system is deleted, the queue is
  //      deleted). Anyway we should still clear all the events
  //      before set_instance(nullptr), and we're not sure if this
  //      is possible on macOS, as some events are queued before the
  //      System instance is even created (see
  //      EventQueue::instance() comment on laf/os/event_queue.h).
  eventQueue()->clearEvents();

  g_instance = nullptr;
}

} // namespace os
