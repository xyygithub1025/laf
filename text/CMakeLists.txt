# LAF Text
# Copyright (C) 2024  Igara Studio S.A.

######################################################################

add_library(laf-text
  draw_text.cpp
  font_mgr.cpp)

target_link_libraries(laf-text laf-os laf-gfx laf-base)

if(FREETYPE_LIBRARIES AND HARFBUZZ_LIBRARIES)
  target_link_libraries(laf-text laf-ft)
  target_compile_definitions(laf-text PRIVATE
    LAF_FREETYPE)
  target_sources(laf-text PRIVATE
    freetype_font.cpp)
endif()

if(LAF_BACKEND STREQUAL "skia")
  target_sources(laf-text PRIVATE
    skia_draw_text.cpp
    skia_draw_text_with_shaper.cpp
    skia_font.cpp
    skia_font_mgr.cpp)
  target_link_libraries(laf-text
    skia skshaper skunicode)
endif()

if(LAF_WITH_TESTS)
  laf_find_tests(. laf-text)
endif()
