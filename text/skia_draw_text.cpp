// LAF Text Library
// Copyright (C) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "text/draw_text.h"

#include "os/paint.h"
#include "os/skia/skia_helpers.h"
#include "os/skia/skia_surface.h"
#include "os/system.h"
#include "text/font_mgr.h"
#include "text/skia_font.h"
#include "text/skia_text_blob.h"

#include "include/core/SkCanvas.h"

namespace text {

void draw_text(
  os::Surface* surface,
  const FontRef& font,
  const std::string& text,
  gfx::PointF pos,
  const os::Paint* paint,
  const TextAlign textAlign)
{
  ASSERT(surface);
  if (!surface)
    return;

  TextBlobRef blob = TextBlob::Make(font, text);
  if (!blob)
    return;

  switch (textAlign) {
    case TextAlign::Left: break;
    case TextAlign::Center: pos.x -= blob->bounds().w / 2.0f; break;
    case TextAlign::Right: pos.x -= blob->bounds().w; break;
  }

  draw_text(surface, blob, pos, paint);
}

void draw_text(
  os::Surface* surface,
  const TextBlobRef& blob,
  const gfx::PointF& pos,
  const os::Paint* paint)
{
  ASSERT(surface);
  if (!surface)
    return;

  static_cast<os::SkiaSurface*>(surface)
    ->canvas().drawTextBlob(
      static_cast<text::SkiaTextBlob*>(blob.get())->skTextBlob(),
      pos.x, pos.y,
      (paint ? paint->skPaint(): SkPaint()));
}

} // namespace text
