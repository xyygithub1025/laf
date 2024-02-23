// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_FWD_H_INCLUDED
#define LAF_TEXT_FWD_H_INCLUDED
#pragma once

#include "base/ref.h"

namespace text {

  class Font;
  using FontRef = base::Ref<Font>;

  class FontMetrics;

  class FontMgr;
  using FontMgrRef = base::Ref<FontMgr>;

  class FontStyle;
  using FontStyleRef = base::Ref<FontStyle>;

  class FontStyleSet;
  using FontStyleSetRef = base::Ref<FontStyleSet>;

  class TextBlob;
  using TextBlobRef = base::Ref<TextBlob>;

  class Typeface;
  using TypefaceRef = base::Ref<Typeface>;

  using GlyphID = uint16_t;

} // namespace text

#endif
