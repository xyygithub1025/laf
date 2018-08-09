# LAF
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

option(LAF_WITH_TESTS "Enable LAF tests" ON)
option(LAF_GFX "Compile laf-gfx library" ON)
option(LAF_OS "Compile laf-os library" ON)
option(LAF_OS_NONE_BACKEND "Compile laf-os library without back-end" OFF)
option(LAF_OS_SKIA_BACKEND "Compile laf-os library with Skia back-end" ON)
option(LAF_OS_WITH_GTK "Enable support for the experimental native GTK File Dialog" OFF)

if(WIN32)
  # We only support the Unicode API
  add_definitions(-DUNICODE -D_UNICODE)
endif()

# Testing
if(LAF_WITH_TESTS)
  enable_testing()
  include(LafFindTests)
endif()

add_subdirectory(third_party)
add_subdirectory(base)
if(LAF_GFX)
  add_subdirectory(gfx)
  if(LAF_OS)
    add_subdirectory(ft)
    add_subdirectory(os)
  endif()
endif()
