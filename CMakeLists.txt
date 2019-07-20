# LAF
# Copyright (C) 2019  Igara Studio S.A.
# Copyright (C) 2016-2018  David Capello

cmake_minimum_required(VERSION 3.2)

project(laf C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Search in the "cmake" directory for additional CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Options
set(LAF_LIST_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "Root LAF CMakeLists.txt file location")
set(LAF_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "Root location of LAF binary files")

if(SKIA_DIR)
  set(LAF_DEFAULT_BACKEND "skia")
else()
  set(LAF_DEFAULT_BACKEND "none")
endif()

option(LAF_WITH_EXAMPLES "Enable LAF examples" ON)
option(LAF_WITH_TESTS "Enable LAF tests" ON)
set(LAF_BACKEND ${LAF_DEFAULT_BACKEND} CACHE STRING "Select laf backend")
set_property(CACHE LAF_BACKEND PROPERTY STRINGS "none" "skia")
if(NOT APPLE AND UNIX)
option(LAF_OS_WITH_GTK "Use GTK library for native dialogs" OFF)
endif()

# Testing
if(LAF_WITH_TESTS)
  enable_testing()
  include(LafFindTests)
endif()

if(LAF_BACKEND STREQUAL "skia")
  include(FindSkia)
endif()

add_subdirectory(third_party)
add_subdirectory(base)
add_subdirectory(gfx)
add_subdirectory(ft)
add_subdirectory(os)
if(LAF_WITH_EXAMPLES)
  add_subdirectory(examples)
endif()

if(LAF_BACKEND STREQUAL "skia")
  target_compile_definitions(laf-base PUBLIC LAF_SKIA)
endif()

# Information

if(NOT LAF_CONF_PRINTED)
  set(LAF_CONF_PRINTED TRUE CACHE INTERNAL "")
  message(
    "=== LAF CONFIGURATION ===\n"
    " Backend:  ${LAF_BACKEND}\n"
    " Pixman:   ${PIXMAN_LIBRARY}\n"
    " FreeType: ${LAF_FREETYPE_LIBRARIES}\n"
    " HarfBuzz: ${LAF_HARFBUZZ_LIBRARIES}\n"
    "=========================")
  if(LAF_BACKEND STREQUAL "skia")
    message(" Skia dir: ${SKIA_DIR}")
  else()
    if(NOT SKIA_DIR)
      message(WARNING "Specify SKIA_DIR if you want to compile laf with the Skia backend")
    endif()
  endif()
endif()
