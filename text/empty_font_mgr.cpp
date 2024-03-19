// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "text/font_mgr.h"

#include "gfx/rect.h"
#include "text/font.h"
#include "text/font_style.h"
#include "text/font_style_set.h"
#include "text/typeface.h"

namespace text {

class EmptyTypeface : public Typeface {
public:
  EmptyTypeface() { }
  FontStyle fontStyle() const override { return FontStyle(); }
};

class EmptyFont : public Font {
public:
  EmptyFont() { }
  FontType type() override { return FontType::Unknown; }
  float metrics(FontMetrics* metrics) const { return 0.0f; }
  int height() const override { return 0; }
  int textLength(const std::string& str) const override { return 0; };
  gfx::RectF measureText(const std::string& str) const override { return gfx::RectF(); }
  bool isScalable() const override { return false; }
  void setSize(int size) override { }
  void setAntialias(bool antialias) override { }
  bool hasCodePoint(int codepoint) const override { return false; }
  gfx::RectF getGlyphBounds(GlyphID glyph) const override { return gfx::RectF(); }
};

class EmptyFontStyleSet : public FontStyleSet {
public:
  EmptyFontStyleSet() { }
  int count() override { return 0; }
  void getStyle(int index,
                FontStyle& style,
                std::string& name) override { }
  TypefaceRef typeface(int index) override {
    return base::make_ref<EmptyTypeface>();
  }
  TypefaceRef matchStyle(const FontStyle& style) override {
    return base::make_ref<EmptyTypeface>();
  }
};

class EmptyFontMgr : public FontMgr {
public:
  EmptyFontMgr() { }
  ~EmptyFontMgr() { }
  FontRef defaultFont(float size) const override {
    return base::make_ref<EmptyFont>();
  }
  int countFamilies() const override { return 0; }
  std::string familyName(int i) const override { return std::string(); }
  FontStyleSetRef familyStyleSet(int i) const override {
    return base::make_ref<EmptyFontStyleSet>();
  }
  FontStyleSetRef matchFamily(const std::string& familyName) const override {
    return base::make_ref<EmptyFontStyleSet>();
  }
};

// static
FontMgrRef FontMgr::Make()
{
  return base::make_ref<EmptyFontMgr>();
}

} // namespace text
