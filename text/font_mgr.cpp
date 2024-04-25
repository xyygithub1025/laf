// LAF Text Library
// Copyright (C) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "text/font_mgr.h"

#include "os/system.h"
#if LAF_FREETYPE
  #include "ft/lib.h"
  #include "text/freetype_font.h"
#endif
#include "text/sprite_sheet_font.h"

namespace text {

FontMgr::FontMgr()
{
}

FontMgr::~FontMgr()
{
}

FontRef FontMgr::loadSpriteSheetFont(const char* filename, int scale)
{
  os::SurfaceRef sheet = os::System::instance()->loadRgbaSurface(filename);
  FontRef font = nullptr;
  if (sheet) {
    sheet->applyScale(scale);
    sheet->setImmutable();
    font = SpriteSheetFont::FromSurface(sheet);
  }
  return font;
}

FontRef FontMgr::loadTrueTypeFont(const char* filename, float size)
{
#if LAF_FREETYPE
  if (!m_ft)
    m_ft.reset(new ft::Lib());
  return FreeTypeFont::LoadFont(*m_ft.get(), filename, size);
#else
  return nullptr;
#endif
}

} // namespace text
