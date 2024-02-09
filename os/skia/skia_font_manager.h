// LAF OS Library
// Copyright (c) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_SKIA_SKIA_FONT_MANAGER_INCLUDED
#define OS_SKIA_SKIA_FONT_MANAGER_INCLUDED
#pragma once

#include "os/font_manager.h"

#include "os/font_style.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"

namespace os {

class SkiaTypeface : public Typeface {
public:
  SkiaTypeface(sk_sp<SkTypeface> skTypeface);

  FontStyle fontStyle() const override;

private:
  sk_sp<SkTypeface> m_skTypeface;
};

class SkiaFontStyleSet : public FontStyleSet {
public:
  SkiaFontStyleSet(sk_sp<SkFontStyleSet> set);

  int count() override;
  void getStyle(int index,
                FontStyle& style,
                std::string& name) override;
  TypefaceRef typeface(int index) override;
  TypefaceRef matchStyle(const FontStyle& style) override;

private:
  sk_sp<SkFontStyleSet> m_skSet;
};

class SkiaFontManager : public FontManager {
public:
  SkiaFontManager();
  ~SkiaFontManager();

  Ref<Font> defaultFont(float size) const override;

  int countFamilies() const override;
  std::string familyName(int i) const override;
  Ref<FontStyleSet> familyStyleSet(int i) const override;
  Ref<FontStyleSet> matchFamily(const std::string& familyName) const override;

  sk_sp<SkFontMgr> skFontMgr() const { return m_skFontMgr; }

private:
  sk_sp<SkFontMgr> m_skFontMgr;
};

} // namespace os

#endif
