// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "text/text_blob.h"

#include "gfx/rect.h"
#include "text/font.h"
#include "text/font_metrics.h"

namespace text {

namespace {

class BoundsBuilder : public TextBlob::RunHandler {
public:
  const gfx::RectF& bounds() const { return m_bounds; }

  // TextBlob::RunHandler impl
  void commitRunBuffer(TextBlob::RunInfo& info) override {
    for (int i = 0; i < info.glyphCount; ++i)
      m_bounds |= info.getGlyphBounds(i);
  }

private:
  gfx::RectF m_bounds;
};

} // namespace anonymous

gfx::RectF TextBlob::bounds()
{
  if (!m_bounds.isEmpty())
    return m_bounds;

  BoundsBuilder handler;
  visitRuns(&handler);
  m_bounds = handler.bounds();
  return m_bounds;
}

TextBlob::Utf8Range TextBlob::RunInfo::getGlyphUtf8Range(size_t i) const
{
  Utf8Range subRange;

  ASSERT(i < glyphCount);
  if (i >= glyphCount)
    return subRange;

  // LTR
  if (!rtl) {
    subRange.begin = utf8Range.begin + clusters[i];
    subRange.end = (i+1 < glyphCount ?
                    utf8Range.begin + clusters[i+1]:
                    utf8Range.end);
  }
  // RTL
  else {
    subRange.begin = utf8Range.begin + clusters[i];
    subRange.end = (i == 0 ? utf8Range.end:
                             utf8Range.begin + clusters[i-1]);
  }
  return subRange;
}

gfx::RectF TextBlob::RunInfo::getGlyphBounds(const size_t i) const
{
  ASSERT(i < glyphCount);
  if (i >= glyphCount)
    return gfx::RectF();

  gfx::RectF bounds = font->getGlyphBounds(glyphs[i]);

  // Get bounds of whitespace
  if (bounds.isEmpty()) {
    FontMetrics metrics;
    font->metrics(&metrics);
    bounds.w = metrics.avgCharWidth;
    bounds.h = -metrics.capHeight;
  }

  bounds.offset(positions[i].x,
                positions[i].y);
  if (offsets) {
    bounds.offset(offsets[i].x,
                  offsets[i].y);
  }
  return bounds;
}

} // namespace text
