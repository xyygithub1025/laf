// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "text/skia_text_blob.h"

#include "text/skia_font.h"
#include "text/skia_font_mgr.h"

#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"

#include <limits>

namespace text {

SkiaTextBlob::SkiaTextBlob(sk_sp<SkTextBlob> skTextBlob)
  : m_skTextBlob(skTextBlob)
{
}

TextBlobRef TextBlob::Make(
  const FontRef& font,
  const std::string& text)
{
  SkFont skFont = static_cast<SkiaFont*>(font.get())->skFont();
  sk_sp<SkTextBlob> textBlob;
  textBlob = SkTextBlob::MakeFromText(text.c_str(), text.size(),
                                      skFont, SkTextEncoding::kUTF8);
  return base::make_ref<SkiaTextBlob>(textBlob);
}

} // namespace text
