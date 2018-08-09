# LAF GFX Library
# Copyright (C) 2001-2017  David Capello

if(NOT PIXMAN_LIBRARY)
  find_library(PIXMAN_LIBRARY NAMES pixman pixman-1)
  find_path(PIXMAN_INCLUDE_DIR NAMES pixman.h PATH_SUFFIXES pixman-1)
endif()

set(LAF_GFX_EXTRA_SOURCES)
if(PIXMAN_LIBRARY)
  set(LAF_GFX_EXTRA_SOURCES region.cpp)
endif()

add_library(laf-gfx
  hsl.cpp
  hsv.cpp
  packing_rects.cpp
  rgb.cpp
  ${LAF_GFX_EXTRA_SOURCES})

target_link_libraries(laf-gfx laf-base)
if(PIXMAN_LIBRARY)
  target_link_libraries(laf-gfx ${PIXMAN_LIBRARY})
  target_include_directories(laf-gfx PRIVATE ${PIXMAN_INCLUDE_DIR})
endif()

if(LAF_WITH_TESTS)
  laf_find_tests(. laf-gfx)
  if(PIXMAN_LIBRARY)
    target_compile_definitions(region_tests
      PRIVATE -DHAVE_PIXMAN)
  endif()
endif()
