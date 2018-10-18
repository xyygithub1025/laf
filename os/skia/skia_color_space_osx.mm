// LAF OS Library
// Copyright (C) 2018  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/color_space.h"
#include "os/system.h"

#include <Cocoa/Cocoa.h>

namespace os {

os::ColorSpacePtr screen_color_space(NSScreen* screen)
{
  os::ColorSpacePtr osCS;
  CGColorSpaceRef cgCS = [[screen colorSpace] CGColorSpace];
  if (cgCS) {
    CFDataRef icc = CGColorSpaceCopyICCProfile(cgCS);
    if (icc) {
      auto gfxCS = gfx::ColorSpace::MakeICC(CFDataGetBytePtr(icc),
                                            CFDataGetLength(icc));

      gfxCS->setName(
        std::string("Display ICC Profile: ") +
        screen.colorSpace.localizedName.UTF8String);

      osCS = os::instance()->createColorSpace(gfxCS);
      CFRelease(icc);
    }
  }
  return osCS;
}

os::ColorSpacePtr main_screen_color_space()
{
  return screen_color_space([NSScreen mainScreen]);
}

void list_screen_color_spaces(std::vector<os::ColorSpacePtr>& list)
{
  // One color profile for each screen
  for (NSScreen* screen in [NSScreen screens]) {
    os::ColorSpacePtr osCS = screen_color_space(screen);
    if (osCS)
      list.push_back(osCS);
  }
}

} // namespace os
