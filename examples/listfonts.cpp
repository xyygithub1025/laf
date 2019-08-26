// LAF Library
// Copyright (c) 2019  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "os/os.h"

#include <cstdio>

static const char* to_str(os::FontStyle::Weight weight)
{
  switch (weight) {
    case os::FontStyle::Weight::Invisible: return "Invisible";
    case os::FontStyle::Weight::Thin: return "Thin";
    case os::FontStyle::Weight::ExtraLight: return "ExtraLight";
    case os::FontStyle::Weight::Light: return "Light";
    case os::FontStyle::Weight::Normal: return "Normal";
    case os::FontStyle::Weight::Medium: return "Medium";
    case os::FontStyle::Weight::SemiBold: return "SemiBold";
    case os::FontStyle::Weight::Bold: return "Bold";
    case os::FontStyle::Weight::ExtraBold: return "ExtraBold";
    case os::FontStyle::Weight::Black: return "Black";
    case os::FontStyle::Weight::ExtraBlack: return "ExtraBlack";
  }
  return "";
}

static const char* to_str(os::FontStyle::Width width)
{
  switch (width) {
    case os::FontStyle::Width::UltraCondensed: return "UltraCondensed";
    case os::FontStyle::Width::ExtraCondensed: return "ExtraCondensed";
    case os::FontStyle::Width::Condensed: return "Condensed";
    case os::FontStyle::Width::SemiCondensed: return "SemiCondensed";
    case os::FontStyle::Width::Normal: return "Normal";
    case os::FontStyle::Width::SemiExpanded: return "SemiExpanded";
    case os::FontStyle::Width::Expanded: return "Expanded";
    case os::FontStyle::Width::ExtraExpanded: return "ExtraExpanded";
    case os::FontStyle::Width::UltraExpanded: return "UltraExpanded";
  }
  return "";
}

static const char* to_str(os::FontStyle::Slant slant)
{
  switch (slant) {
    case os::FontStyle::Slant::Upright: return "Upright";
    case os::FontStyle::Slant::Italic: return "Italic";
    case os::FontStyle::Slant::Oblique: return "Oblique";
  }
  return "";
}

int app_main(int argc, char* argv[])
{
  os::SystemHandle system(os::create_system());
  auto fm = system->fontManager();
  if (!fm) {
    std::printf("There is no font manager in your platform\n");
    return 1;
  }

  const int n = fm->countFamilies();
  for (int i=0; i<n; ++i) {
    std::string name = fm->familyName(i);
    std::printf("%s\n", name.c_str());

    auto fnset = fm->matchFamily(name);
    auto set = fm->familyStyleSet(i);
    assert(fnset->count() == set->count());

    for (int j=0; j<set->count(); ++j) {
      os::FontStyle style;
      std::string styleName;
      set->getStyle(j, style, styleName);
      std::printf(" * %s (%s %s %s)\n",
                  name.c_str(),
                  to_str(style.weight()),
                  to_str(style.width()),
                  to_str(style.slant()));

      auto face = set->typeface(j);
      assert(face->fontStyle() == style);
    }
  }

  return 0;
}
