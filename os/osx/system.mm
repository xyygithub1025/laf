// LAF OS Library
// Copyright (c) 2020-2021  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/osx/system.h"

#include "os/osx/screen.h"

namespace os {

ScreenRef SystemOSX::mainScreen()
{
  return make_ref<ScreenOSX>([NSScreen mainScreen]);
}

void SystemOSX::listScreens(ScreenList& list)
{
  auto screens = [NSScreen screens];
  for (NSScreen* screen : screens)
    list.push_back(make_ref<ScreenOSX>(screen));
}

}
