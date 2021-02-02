// LAF OS Library
// Copyright (c) 2018-2021  Igara Studio S.A.
// Copyright (C) 2012-2015  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_CAPABILITIES_H_INCLUDED
#define OS_CAPABILITIES_H_INCLUDED
#pragma once

namespace os {

  enum class Capabilities {
    MultipleWindows = 1,
    CanResizeWindow = 2,
    WindowScale = 4,
    CustomNativeMouseCursor = 8,
    GpuAccelerationSwitch = 16,
    ColorSpaces = 32
  };

} // namespace os

#endif
