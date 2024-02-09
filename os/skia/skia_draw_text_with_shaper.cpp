// LAF OS Library
// Copyright (C) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/draw_text.h"
#include "os/paint.h"
#include "os/skia/skia_font.h"
#include "os/skia/skia_font_manager.h"
#include "os/skia/skia_helpers.h"
#include "os/skia/skia_surface.h"
#include "os/system.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkTextBlob.h"
#include "include/utils/SkTextUtils.h"
#include "modules/skshaper/include/SkShaper.h"

#include <cfloat>

namespace os {

void draw_text_with_shaper(
  Surface* surface,
  FontRef font,
  const std::string& text,
  const gfx::Point& pos,
  const Paint* paint,
  const TextAlign textAlign,
  DrawTextDelegate* delegate)
{
  auto fontMgr = static_cast<SkiaFontManager*>(os::instance()->fontManager());
  if (!fontMgr)
    return;

  if (!font)
    font = fontMgr->defaultFont(12);

  if (font->type() != FontType::Native)
    return;

  SkFont skFont = static_cast<SkiaFont*>(font.get())->skFont();
  sk_sp<SkTextBlob> textBlob;
  if (auto shaper = SkShaper::Make(fontMgr->skFontMgr())) {
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
    static_cast<SkiaSurface*>(surface)->canvas()
      .drawTextBlob(
        textBlob,
        SkIntToScalar(pos.x),
        SkIntToScalar(pos.y),
        (paint ? paint->skPaint(): SkPaint()));
  }
}

} // namespace os
