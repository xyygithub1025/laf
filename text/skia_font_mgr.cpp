// LAF Text Library
// Copyright (c) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "text/skia_font_mgr.h"

#include "text/skia_font.h"

#include "include/core/SkFont.h"
#include "include/core/SkString.h"

#if LAF_WINDOWS
  #include "include/ports/SkTypeface_win.h"
#elif LAF_MACOS
  #include "include/ports/SkFontMgr_mac_ct.h"
#elif LAF_LINUX
  #include "include/ports/SkFontMgr_fontconfig.h"
#endif

namespace text {

//////////////////////////////////////////////////////////////////////
// SkiaTypeface

SkiaTypeface::SkiaTypeface(sk_sp<SkTypeface> skTypeface)
  : m_skTypeface(skTypeface)
{
}

std::string SkiaTypeface::familyName() const
{
  SkString name;
  m_skTypeface->getFamilyName(&name);
  return std::string(name.c_str());
}

FontStyle SkiaTypeface::fontStyle() const
{
  SkFontStyle skStyle = m_skTypeface->fontStyle();
  return FontStyle((FontStyle::Weight)skStyle.weight(),
                   (FontStyle::Width)skStyle.width(),
                   (FontStyle::Slant)skStyle.slant());
}

//////////////////////////////////////////////////////////////////////
// SkiaFontStyleSet

SkiaFontStyleSet::SkiaFontStyleSet(sk_sp<SkFontStyleSet> set)
  : m_skSet(set)
{
}

int SkiaFontStyleSet::count()
{
  return m_skSet->count();
}

void SkiaFontStyleSet::getStyle(int index,
                                FontStyle& style,
                                std::string& name)
{
  SkFontStyle skStyle;
  SkString skName;
  m_skSet->getStyle(index, &skStyle, &skName);
  style = FontStyle((FontStyle::Weight)skStyle.weight(),
                    (FontStyle::Width)skStyle.width(),
                    (FontStyle::Slant)skStyle.slant());
  name = skName.c_str();
}

TypefaceRef SkiaFontStyleSet::typeface(int index)
{
  return base::make_ref<SkiaTypeface>(m_skSet->createTypeface(index));
}

TypefaceRef SkiaFontStyleSet::matchStyle(const FontStyle& style)
{
  SkFontStyle skStyle((SkFontStyle::Weight)style.weight(),
                      (SkFontStyle::Width)style.width(),
                      (SkFontStyle::Slant)style.slant());
  return base::make_ref<SkiaTypeface>(m_skSet->matchStyle(skStyle));
}

//////////////////////////////////////////////////////////////////////
// SkiaFontMgr

// static
FontMgrRef FontMgr::Make()
{
  return base::make_ref<SkiaFontMgr>();
}

SkiaFontMgr::SkiaFontMgr()
{
#if LAF_WINDOWS
  m_skFontMgr = SkFontMgr_New_DirectWrite();
#elif LAF_MACOS
  m_skFontMgr = SkFontMgr_New_CoreText(nullptr);
#elif LAF_LINUX
  m_skFontMgr = SkFontMgr_New_FontConfig(nullptr);
#endif
  if (!m_skFontMgr)
    m_skFontMgr = SkFontMgr::RefEmpty();
}

SkiaFontMgr::~SkiaFontMgr()
{
}

FontRef SkiaFontMgr::defaultFont(float size) const
{
  sk_sp<SkTypeface> face =
    m_skFontMgr->legacyMakeTypeface(nullptr, SkFontStyle());
  ASSERT(face);
  SkFont skFont(face, size);
  return base::make_ref<SkiaFont>(skFont);
}

FontRef SkiaFontMgr::makeFont(const TypefaceRef& typeface)
{
  ASSERT(typeface.get());
  return base::make_ref<SkiaFont>(
    SkFont(static_cast<SkiaTypeface*>(typeface.get())->skTypeface()));
}

FontRef SkiaFontMgr::makeFont(const TypefaceRef& typeface, float size)
{
  ASSERT(typeface.get());
  return base::make_ref<SkiaFont>(
    SkFont(static_cast<SkiaTypeface*>(typeface.get())->skTypeface(), size));
}

int SkiaFontMgr::countFamilies() const
{
  return m_skFontMgr->countFamilies();
}

std::string SkiaFontMgr::familyName(int i) const
{
  SkString name;
  m_skFontMgr->getFamilyName(i, &name);
  return std::string(name.c_str());
}

FontStyleSetRef SkiaFontMgr::familyStyleSet(int i) const
{
  return base::make_ref<SkiaFontStyleSet>(m_skFontMgr->createStyleSet(i));
}

FontStyleSetRef SkiaFontMgr::matchFamily(const std::string& familyName) const
{
  return base::make_ref<SkiaFontStyleSet>(m_skFontMgr->matchFamily(familyName.c_str()));
}

} // namespace text
