// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_TEXT_BLOB_H_INCLUDED
#define LAF_TEXT_TEXT_BLOB_H_INCLUDED
#pragma once

#include "gfx/fwd.h"
#include "gfx/point.h"
#include "text/fwd.h"
#include "text/text_blob.h"

#include <functional>
#include <string>

namespace text {

  class TextBlob : public base::RefCount {
  public:
    struct Utf8Range {
      size_t begin = 0;
      size_t end = 0;
    };

    // Based on SkShaper::RunHandler::RunInfo and Buffer
    struct RunInfo {
      FontRef font;
      size_t glyphCount = 0;
      bool rtl = false;
      Utf8Range utf8Range;
      GlyphID* glyphs = nullptr;        // required
      gfx::PointF* positions = nullptr; // required, if (!offsets) put glyphs[i] at positions[i]
                                        // if (offsets) positions[i+1]-positions[i] are advances
      gfx::PointF* offsets = nullptr;   // optional, if ( offsets) put glyphs[i] at positions[i]+offsets[i]
      uint32_t* clusters = nullptr;     // optional, utf8+clusters[i] starts run which produced glyphs[i]
      gfx::PointF point;                // offset to add to all positions

      Utf8Range getGlyphUtf8Range(size_t i) const;
      gfx::RectF getGlyphBounds(size_t i) const;
    };

    class RunHandler {
    public:
      virtual ~RunHandler() { }
      virtual void commitRunBuffer(RunInfo& info) = 0;
    };

    TextBlob() { }
    virtual ~TextBlob() { }

    // Uses Skia'SkTextBlob::MakeFromText() to create the TextBlob, it
    // doesn't depend on harfbuzz or big dependencies, useful to print
    // English text only.
    static TextBlobRef Make(
      const FontRef& font,
      const std::string& text);

    // Uses SkShaper::Make() to draw text (harfbuzz if available),
    // useful for RTL (right-to-left) languages. Avoid this function
    // if you are not going to translate your app to non-English
    // languages (prefer TextBlob::Make() when possible).
    static TextBlobRef MakeWithShaper(
      const FontMgrRef& fontMgr,
      const FontRef& font,
      const std::string& text,
      TextBlob::RunHandler* handler = nullptr);
  };

} // namespace text

#endif
