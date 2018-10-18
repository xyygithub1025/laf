// LAF OS Library
// Copyright (C) 2018  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/color_space.h"

#include <windows.h>

namespace os {

os::ColorSpacePtr main_screen_color_space()
{
  // TODO
  return os::instance()->createColorSpace(gfx::ColorSpace::sRGB);
}

void list_screen_color_spaces(std::vector<os::ColorSpacePtr>& list)
{
  // TODO
}

} // namespace os
