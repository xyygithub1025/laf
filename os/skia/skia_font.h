// LAF OS Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_SKIA_SKIA_FONT_H_INCLUDED
#define OS_SKIA_SKIA_FONT_H_INCLUDED
#pragma once

#include "os/font.h"

#include "include/core/SkFont.h"

namespace os {

  class SkiaFont : public Font {
  public:
    SkiaFont(const SkFont& skFont);
    ~SkiaFont();

    bool isValid() const;
    FontType type() override;
    int height() const override;
    int textLength(const std::string& str) const override;
    bool isScalable() const override;
    void setSize(int size) override;
    void setAntialias(bool antialias) override;
    bool hasCodePoint(int codepoint) const override;

    SkFont& skFont() { return m_skFont; }

  private:
    SkFont m_skFont;
  };

} // namespace os

#endif
