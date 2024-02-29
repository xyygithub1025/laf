// LAF OS Library
// Copyright (C) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/event.h"

#include <cuchar>
#include <vector>

namespace os {

std::string Event::unicodeCharAsUtf8() const
{
  std::vector<char> buf;
  if (m_unicodeChar) {
    std::mbstate_t state;
    buf.resize(8);
    std::size_t size = std::c32rtomb(buf.data(),
                                     m_unicodeChar, &state);
    if (size < 0 || size >= buf.size())
      size = 0;
    buf[size] = 0;
  }
  buf.push_back(0);
  return std::string(buf.data());
}

} // namespace os
