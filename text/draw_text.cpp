// LAF Text Library
// Copyright (C) 2020-2024  Igara Studio S.A.
// Copyright (C) 2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "text/draw_text.h"

#include "gfx/clip.h"
#include "os/common/generic_surface.h"
#include "os/paint.h"
#include "text/sprite_sheet_font.h"
#include "text/text_blob.h"

#if LAF_FREETYPE
  #include "ft/algorithm.h"
  #include "ft/hb_shaper.h"
  #include "text/freetype_font.h"
#endif

#if LAF_SKIA
  #include "os/skia/skia_surface.h"
  #include "text/skia_font.h"

  #include "include/core/SkCanvas.h"
#endif

namespace text {

namespace {

// Adapts the old DrawTextDelegate with new TextBlob run handlers.
class AdapterBuilder : public TextBlob::RunHandler {
public:
  AdapterBuilder(os::Surface* surface,
                 const std::string& text,
                 gfx::Color fg,
                 gfx::Color bg,
                 const gfx::PointF& origin,
                 DrawTextDelegate* delegate)
    : m_surface(surface)
    , m_text(text)
    , m_fg(fg), m_bg(bg)
    , m_origin(origin)
    , m_delegate(delegate) { }

  // TextBlob::RunHandler impl
  void commitRunBuffer(TextBlob::RunInfo& info) override {
    if (info.clusters &&
        info.glyphCount > 0) {
      os::Paint paint;
      paint.style(os::Paint::Fill);

      for (int i=0; i<info.glyphCount; ++i) {
        int utf8Begin, utf8End;

        // LTR
        if (!info.rtl) {
          utf8Begin = info.utf8Range.begin + info.clusters[i];
          utf8End = (i+1 < info.glyphCount ?
                     info.utf8Range.begin + info.clusters[i+1]:
                     info.utf8Range.end);
        }
        // RTL
        else {
          utf8Begin = info.utf8Range.begin + info.clusters[i];
          utf8End = (i == 0 ? info.utf8Range.end:
                              info.utf8Range.begin + info.clusters[i-1]);
        }

        const std::string utf8text =
          m_text.substr(utf8Begin, utf8End - utf8Begin);

        gfx::RectF bounds = info.getGlyphBounds(i);
        bounds.offset(m_origin.x, m_origin.y);

        if (m_delegate) {
          const std::wstring widetext = base::from_utf8(utf8text);
          base::codepoint_t codepoint = 0;
          if (widetext.size() > 0) {
            // On macOS and Linux wchar_t has 32-bits
            if constexpr (sizeof(wchar_t) >= 4) {
              codepoint = widetext[0];
            }
            // On Windows wchar_t has 16-bits (wide strings are UTF-16 strings)
            else if constexpr (sizeof(wchar_t) == 2) {
              codepoint = base::utf16_to_codepoint(
                widetext.size() > 1 ? widetext[1]: widetext[0],
                widetext.size() > 1 ? widetext[0]: 0);
            }
            else {
              codepoint = 0;
            }
          }

          m_delegate->preProcessChar(utf8Begin, codepoint,
                                     m_fg, m_bg, bounds);
        }

        if (m_delegate)
          m_delegate->preDrawChar(bounds);

        if (m_bg != gfx::ColorNone) {
          paint.color(m_bg);
          m_surface->drawRect(bounds, paint);
        }

#if LAF_SKIA
        if (m_surface) {
          SkGlyphID glyphs = info.glyphs[i];
          SkPoint positions = SkPoint::Make(info.positions[i].x,
                                            info.positions[i].y);
          uint32_t clusters = info.clusters[i];
          paint.color(m_fg);
          static_cast<os::SkiaSurface*>(m_surface)
            ->canvas().drawGlyphs(
              1, &glyphs, &positions, &clusters,
              utf8text.size(),
              utf8text.data(),
              SkPoint::Make(m_origin.x, m_origin.y),
              static_cast<SkiaFont*>(info.font.get())->skFont(),
              paint.skPaint());
        }
#endif

        if (m_delegate)
          m_delegate->postDrawChar(bounds);
      }
    }
  }

private:
  os::Surface* m_surface;
  const std::string& m_text;
  gfx::Color m_fg;
  gfx::Color m_bg;
  gfx::PointF m_origin;
  DrawTextDelegate* m_delegate;
};

}

gfx::Rect draw_text(os::Surface* surface,
                    const FontMgrRef& fontMgr,
                    const FontRef& fontRef,
                    const std::string& text,
                    gfx::Color fg, gfx::Color bg,
                    int x, int y,
                    DrawTextDelegate* delegate)
{
  Font* font = fontRef.get();

  base::utf8_decode decode(text);
  gfx::Rect textBounds;

retry:;
  // Check if this font is enough to draw the given string or we will
  // need the fallback for some special Unicode chars
  if (font->fallback()) {
    // TODO compose unicode characters and check those codepoints, the
    //      same in the drawing code of sprite sheet font
    while (const uint32_t code = decode.next()) {
      if (code && !font->hasCodePoint(code)) {
        Font* newFont = font->fallback();

        // Search a valid fallback
        while (newFont && !newFont->hasCodePoint(code))
          newFont = newFont->fallback();
        if (!newFont)
          break;

        y += font->height()/2 - newFont->height()/2;

        font = newFont;
        goto retry;
      }
    }
  }

  switch (font->type()) {

    case FontType::Unknown:
      // Do nothing
      break;

    case FontType::SpriteSheet: {
      SpriteSheetFont* ssFont = static_cast<SpriteSheetFont*>(font);
      os::Surface* sheet = ssFont->sheetSurface();

      if (surface) {
        sheet->lock();
        surface->lock();
      }

      decode = base::utf8_decode(text);
      while (true) {
        const int i = decode.pos() - text.begin();
        const int chr = decode.next();
        if (!chr)
          break;

        const gfx::Rect charBounds = ssFont->getCharBounds(chr);
        const gfx::Rect outCharBounds(x, y, charBounds.w, charBounds.h);

        if (delegate)
          delegate->preProcessChar(i, chr, fg, bg, outCharBounds);

        if (delegate && !delegate->preDrawChar(outCharBounds))
          break;

        if (!charBounds.isEmpty()) {
          if (surface)
            surface->drawColoredRgbaSurface(sheet, fg, bg, gfx::Clip(x, y, charBounds));
        }

        textBounds |= outCharBounds;
        if (delegate)
          delegate->postDrawChar(outCharBounds);

        x += charBounds.w;
      }

      if (surface) {
        surface->unlock();
        sheet->unlock();
      }
      break;
    }

    case FontType::FreeType: {
#if LAF_FREETYPE
      FreeTypeFont* ttFont = static_cast<FreeTypeFont*>(font);
      const int fg_alpha = gfx::geta(fg);

      gfx::Rect clipBounds;
      os::SurfaceFormatData fd;
      if (surface) {
        clipBounds = surface->getClipBounds();
        surface->getFormat(&fd);
        surface->lock();
      }

      ft::ForEachGlyph<FreeTypeFont::Face> feg(ttFont->face(), text);
      while (feg.next()) {
        gfx::Rect origDstBounds;
        const auto* glyph = feg.glyph();
        if (glyph)
          origDstBounds = gfx::Rect(
            x + int(glyph->startX),
            y + int(glyph->y),
            int(glyph->endX) - int(glyph->startX),
            int(glyph->bitmap->rows) ? int(glyph->bitmap->rows): 1);

        if (delegate) {
          delegate->preProcessChar(
            feg.charIndex(),
            feg.unicodeChar(),
            fg, bg, origDstBounds);
        }

        if (!glyph)
          continue;

        if (delegate && !delegate->preDrawChar(origDstBounds))
          break;

        origDstBounds.x = x + int(glyph->x);
        origDstBounds.w = int(glyph->bitmap->width);
        origDstBounds.h = int(glyph->bitmap->rows);

        gfx::Rect dstBounds = origDstBounds;
        if (surface)
          dstBounds &= clipBounds;

        if (surface && !dstBounds.isEmpty()) {
          const int clippedRows = dstBounds.y - origDstBounds.y;
          int dst_y = dstBounds.y;
          int t;
          for (int v=0; v<dstBounds.h; ++v, ++dst_y) {
            int bit = 0;
            const uint8_t* p = glyph->bitmap->buffer
              + (v+clippedRows)*glyph->bitmap->pitch;
            int dst_x = dstBounds.x;
            uint32_t* dst_address =
              (uint32_t*)surface->getData(dst_x, dst_y);

            // TODO maybe if we are trying to draw in a SkiaSurface with a nullptr m_bitmap
            //      (when GPU-acceleration is enabled)
            if (!dst_address)
              break;

            // Skip first clipped pixels
            for (int u=0; u<dstBounds.x-origDstBounds.x; ++u) {
              if (glyph->bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
                ++p;
              }
              else if (glyph->bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
                if (++bit == 8) {
                  bit = 0;
                  ++p;
                }
              }
            }

            for (int u=0; u<dstBounds.w; ++u, ++dst_x) {
              ASSERT(clipBounds.contains(gfx::Point(dst_x, dst_y)));

              int alpha;
              if (glyph->bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
                alpha = *(p++);
              }
              else if (glyph->bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
                alpha = ((*p) & (1 << (7 - (bit++))) ? 255: 0);
                if (bit == 8) {
                  bit = 0;
                  ++p;
                }
              }
              else
                alpha = 0;

              const uint32_t backdrop = *dst_address;
              const gfx::Color backdropColor =
                gfx::rgba(
                  ((backdrop & fd.redMask) >> fd.redShift),
                  ((backdrop & fd.greenMask) >> fd.greenShift),
                  ((backdrop & fd.blueMask) >> fd.blueShift),
                  ((backdrop & fd.alphaMask) >> fd.alphaShift));

              gfx::Color output = gfx::rgba(gfx::getr(fg),
                                            gfx::getg(fg),
                                            gfx::getb(fg),
                                            MUL_UN8(fg_alpha, alpha, t));
              if (gfx::geta(bg) > 0)
                output = os::blend(os::blend(backdropColor, bg), output);
              else
                output = os::blend(backdropColor, output);

              *dst_address =
                ((gfx::getr(output) << fd.redShift  ) & fd.redMask  ) |
                ((gfx::getg(output) << fd.greenShift) & fd.greenMask) |
                ((gfx::getb(output) << fd.blueShift ) & fd.blueMask ) |
                ((gfx::geta(output) << fd.alphaShift) & fd.alphaMask);

              ++dst_address;
            }
          }
        }

        if (!origDstBounds.w) origDstBounds.w = 1;
        if (!origDstBounds.h) origDstBounds.h = 1;
        textBounds |= origDstBounds;
        if (delegate)
          delegate->postDrawChar(origDstBounds);
      }

      if (surface)
        surface->unlock();
#endif // LAF_FREETYPE
      break;
    }

    case FontType::Native: {
#if LAF_SKIA
      TextBlobRef blob;
      if (delegate) {
        AdapterBuilder handler(surface, text, fg, bg,
                               gfx::PointF(x, y), delegate);
        blob = TextBlob::MakeWithShaper(fontMgr, fontRef, text, &handler);
      }
      else {
        os::Paint paint;
        paint.color(fg);
        // TODO Draw background with bg
        blob = TextBlob::MakeWithShaper(fontMgr, fontRef, text);
        draw_text(surface, blob, gfx::PointF(x, y), &paint);
      }
#endif // LAF_SKIA
      break;
    }

  }

  return textBounds;
}

} // namespace text
