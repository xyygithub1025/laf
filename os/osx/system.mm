// LAF OS Library
// Copyright (c) 2020  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/osx/system.h"

#include "os/osx/screen.h"

namespace os {

ScreenRef OSXSystem::mainScreen()
{
  return make_ref<OSXScreen>([NSScreen mainScreen]);
}

void OSXSystem::listScreens(ScreenList& list)
{
  auto screens = [NSScreen screens];
  for (NSScreen* screen : screens)
    list.push_back(make_ref<OSXScreen>(screen));
}

}
