// LAF Text Library
// Copyright (C) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/paint.h"
#include "os/skia/skia_helpers.h"
#include "os/skia/skia_surface.h"
#include "os/system.h"
#include "text/draw_text.h"
#include "text/skia_font.h"
#include "text/skia_font_mgr.h"
#include "text/skia_text_blob.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkTextBlob.h"
#include "include/utils/SkTextUtils.h"
#include "modules/skshaper/include/SkShaper.h"

#include <limits>

namespace text {

namespace {

class ShaperRunHandler final : public SkShaper::RunHandler {
public:
  ShaperRunHandler(const char* utf8Text, SkPoint offset,
                   TextBlob::RunHandler* subHandler)
    : m_builder(utf8Text, offset)
    , m_subHandler(subHandler) { }

  sk_sp<SkTextBlob> makeBlob() {
    return m_builder.makeBlob();
  }

  void beginLine() override {
    m_builder.beginLine();
  }

  void runInfo(const RunInfo& info) override {
    m_builder.runInfo(info);
  }

  void commitRunInfo() override {
    m_builder.commitRunInfo();
  }

  Buffer runBuffer(const RunInfo& info) override {
    m_buffer = m_builder.runBuffer(info);
    return m_buffer;
  }

  void commitRunBuffer(const RunInfo& info) override {
    SkString family;
    info.fFont.getTypeface()
      ->getFamilyName(&family);

    m_builder.commitRunBuffer(info);

    // Now the m_buffer field is valid and can be used
    size_t n = info.glyphCount;
    TextBlob::RunInfo subInfo;
    FontRef font = base::make_ref<SkiaFont>(info.fFont);
    subInfo.font = font;
    subInfo.glyphCount = n;
    subInfo.rtl = (info.fBidiLevel & 1);
    subInfo.utf8Range.begin = info.utf8Range.begin();
    subInfo.utf8Range.end = info.utf8Range.end();
    subInfo.glyphs = m_buffer.glyphs;

    if (m_positions.size() < n)
      m_positions.resize(n);
    for (size_t i=0; i<n; ++i) {
      m_positions[i] = gfx::PointF(m_buffer.positions[i].x(),
                                   m_buffer.positions[i].y());
    }
    subInfo.positions = m_positions.data();

    if (m_buffer.offsets) {
      if (m_offsets.size() < n)
        m_offsets.resize(n);
      for (size_t i=0; i<n; ++i) {
        m_offsets[i] = gfx::PointF(m_buffer.offsets[i].x(),
                                   m_buffer.offsets[i].y());
      }
      subInfo.offsets = m_offsets.data();
    }

    subInfo.clusters = m_buffer.clusters;
    subInfo.point = gfx::PointF(m_buffer.point.x(),
                                m_buffer.point.y());

    if (m_subHandler)
      m_subHandler->commitRunBuffer(subInfo);
  }

  void commitLine() override {
    m_builder.commitLine();
  }

private:
  SkTextBlobBuilderRunHandler m_builder;
  TextBlob::RunHandler* m_subHandler;
  Buffer m_buffer;
  std::vector<gfx::PointF> m_positions;
  std::vector<gfx::PointF> m_offsets;
};

}

TextBlobRef TextBlob::MakeWithShaper(
  const FontMgrRef& fontMgr,
  const FontRef& font,
  const std::string& text,
  TextBlob::RunHandler* handler)
{
  SkFont skFont = static_cast<SkiaFont*>(font.get())->skFont();
  sk_sp<SkTextBlob> textBlob;
  if (auto shaper = SkShaper::Make(
        static_cast<SkiaFontMgr*>(fontMgr.get())->skFontMgr())) {
    ShaperRunHandler shaperHandler(text.c_str(), { 0, 0 }, handler);
    shaper->shape(
      text.c_str(), text.size(),
      skFont,
      true,
      std::numeric_limits<float>::max(),
      &shaperHandler);
    textBlob = shaperHandler.makeBlob();
  }
  else {
    textBlob = SkTextBlob::MakeFromText(text.c_str(), text.size(),
                                        skFont, SkTextEncoding::kUTF8);
  }

  return base::make_ref<SkiaTextBlob>(textBlob);
}

void draw_text_with_shaper(
  os::Surface* surface,
  const FontMgrRef& fontMgr,
  const FontRef& font,
  const std::string& text,
  const gfx::PointF& pos,
  const os::Paint* paint,
  const TextAlign textAlign,
  DrawTextDelegate* delegate)
{
  if (!fontMgr || !font || font->type() != FontType::Native)
    return;

  // TODO call "delegate" then
  TextBlobRef blob = TextBlob::MakeWithShaper(
    fontMgr, font, text, nullptr);
  if (blob)
    draw_text(surface, blob, pos, paint);
}

} // namespace text
