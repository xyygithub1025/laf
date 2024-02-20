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

#include "include/core/SkCanvas.h"
#include "include/core/SkTextBlob.h"
#include "include/utils/SkTextUtils.h"
#include "modules/skshaper/include/SkShaper.h"

#include <cfloat>

namespace text {

void draw_text_with_shaper(
  os::Surface* surface,
  const FontMgrRef& fontMgr,
  const FontRef& font,
  const std::string& text,
  const gfx::Point& pos,
  const os::Paint* paint,
  const TextAlign textAlign,
  DrawTextDelegate* delegate)
{
  if (!fontMgr || !font || font->type() != FontType::Native)
    return;

  SkFont skFont = static_cast<SkiaFont*>(font.get())->skFont();
  sk_sp<SkTextBlob> textBlob;
  if (auto shaper = SkShaper::Make(static_cast<SkiaFontMgr*>(fontMgr.get())->skFontMgr())) {
    SkTextBlobBuilderRunHandler builder(text.c_str(), { 0, 0 });
    shaper->shape(
      text.c_str(), text.size(),
      skFont,
      true,
      FLT_MAX,
      &builder);
    textBlob = builder.makeBlob();
  }
  else {
    textBlob = SkTextBlob::MakeFromText(text.c_str(), text.size(),
                                        skFont, SkTextEncoding::kUTF8);
  }

  if (textBlob) {
    static_cast<os::SkiaSurface*>(surface)->canvas()
      .drawTextBlob(
        textBlob,
        SkIntToScalar(pos.x),
        SkIntToScalar(pos.y),
        (paint ? paint->skPaint(): SkPaint()));
  }
}

} // namespace text
