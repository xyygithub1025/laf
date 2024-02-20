// LAF Text Library
// Copyright (C) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_FONT_MGR_H_INCLUDED
#define LAF_TEXT_FONT_MGR_H_INCLUDED
#pragma once

#include "text/fwd.h"

#include <memory>

namespace ft {
  class Lib;
}

namespace text {

  class FontMgr : public base::RefCount {
  public:
    static base::Ref<FontMgr> Make();

    FontRef loadSpriteSheetFont(const char* filename, int scale);
    FontRef loadTrueTypeFont(const char* filename, int height);

    virtual FontRef defaultFont(float size = 12) const = 0;
    virtual int countFamilies() const = 0;
    virtual std::string familyName(int index) const = 0;
    virtual base::Ref<FontStyleSet> familyStyleSet(int index) const = 0;
    virtual base::Ref<FontStyleSet> matchFamily(const std::string& familyName) const = 0;

  protected:
    FontMgr();
    virtual ~FontMgr();

  private:
#ifdef LAF_FREETYPE
    std::unique_ptr<ft::Lib> m_ft;
#endif
  };

} // namespace text

#endif
