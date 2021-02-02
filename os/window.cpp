// LAF OS Library
// Copyright (C) 2019-2021  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/window.h"

#include "gfx/rect.h"
#include "gfx/region.h"

namespace os {

gfx::Rect Window::bounds() const
{
  return gfx::Rect(0, 0, width(), height());
}

void Window::invalidate()
{
  invalidateRegion(gfx::Region(bounds()));
}

} // namespace os
