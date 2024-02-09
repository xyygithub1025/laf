// LAF OS Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "os/skia/skia_font.h"

#include "include/core/SkFontTypes.h"

namespace os {

SkiaFont::SkiaFont(const SkFont& skFont)
  : m_skFont(skFont)
{
}

SkiaFont::~SkiaFont()
{
}

bool SkiaFont::isValid() const
{
  return true;
}

FontType SkiaFont::type()
{
  return FontType::Native;
}

int SkiaFont::height() const
{
  return m_skFont.getSize();
}

int SkiaFont::textLength(const std::string& str) const
{
  SkRect bounds;
  m_skFont.measureText(
    str.c_str(),
    str.size(),
    SkTextEncoding::kUTF8,
    &bounds);
  return bounds.width();
}

bool SkiaFont::isScalable() const
{
  return true;
}

void SkiaFont::setSize(int size)
{
  m_skFont.setSize(size);
}

void SkiaFont::setAntialias(bool antialias)
{
  m_skFont.setEdging(antialias ? SkFont::Edging::kAntiAlias:
                                 SkFont::Edging::kAlias);
}

bool SkiaFont::hasCodePoint(int codepoint) const
{
  return (m_skFont.unicharToGlyph(codepoint) != 0);
}

} // namespace os
