# Copyright (C) 2019  Igara Studio S.A.
#
# This file is released under the terms of the MIT license.
# Read LICENSE.txt for more information.

set(SKIA_DIR "" CACHE PATH "Skia source code directory")
if(NOT SKIA_DIR)
  set(SKIA_OUT_DIR "" CACHE PATH "Skia output directory")
else()
  if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(SKIA_OUT_DIR "${SKIA_DIR}/out/Debug" CACHE PATH "Skia output directory")
  else()
    set(SKIA_OUT_DIR "${SKIA_DIR}/out/Release" CACHE PATH "Skia output directory")
  endif()
endif()

find_library(SKIA_LIBRARY skia PATH "${SKIA_OUT_DIR}")
add_library(skia INTERFACE)
if(WIN32)
  find_library(SKIA_OPENGL_LIBRARY opengl32)
else()
  find_library(SKIA_OPENGL_LIBRARY opengl NAMES GL)
endif()

find_path(SKIA_CONFIG_INCLUDE_DIR SkUserConfig.h HINTS "${SKIA_DIR}/include/config")
find_path(SKIA_CORE_INCLUDE_DIR SkCanvas.h HINTS "${SKIA_DIR}/include/core")
find_path(SKIA_UTILS_INCLUDE_DIR SkRandom.h HINTS "${SKIA_DIR}/include/utils")
find_path(SKIA_CODEC_INCLUDE_DIR SkCodec.h HINTS "${SKIA_DIR}/include/codec")
find_path(SKIA_EFFECTS_INCLUDE_DIR SkImageSource.h HINTS "${SKIA_DIR}/include/effects")
find_path(SKIA_GPU_INCLUDE_DIR GrContext.h HINTS "${SKIA_DIR}/include/gpu")
find_path(SKIA_GPU2_INCLUDE_DIR gl/GrGLDefines.h HINTS "${SKIA_DIR}/src/gpu")
find_path(SKIA_ANGLE_INCLUDE_DIR angle_gl.h HINTS "${SKIA_DIR}/third_party/externals/angle2/include")
find_path(SKIA_SKCMS_INCLUDE_DIR skcms.h HINTS "${SKIA_DIR}/third_party/skcms")

include_directories(
  ${SKIA_CONFIG_INCLUDE_DIR}
  ${SKIA_CORE_INCLUDE_DIR}
  ${SKIA_PORTS_INCLUDE_DIR}
  ${SKIA_UTILS_INCLUDE_DIR}
  ${SKIA_CODEC_INCLUDE_DIR}
  ${SKIA_GPU_INCLUDE_DIR}
  ${SKIA_GPU2_INCLUDE_DIR}
  ${SKIA_SKCMS_INCLUDE_DIR})
if(WIN32)
  include_directories(${SKIA_ANGLE_INCLUDE_DIR})
endif()

set(SKIA_LIBRARIES
  ${SKIA_LIBRARY}
  ${SKIA_OPENGL_LIBRARY}
  CACHE INTERNAL "Skia libraries")

target_link_libraries(skia INTERFACE ${SKIA_LIBRARIES})

target_compile_definitions(skia INTERFACE
  SK_INTERNAL
  SK_GAMMA_SRGB
  SK_GAMMA_APPLY_TO_A8
  SK_SCALAR_TO_FLOAT_EXCLUDED
  SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1
  SK_SUPPORT_OPENCL=0
  SK_FORCE_DISTANCE_FIELD_TEXT=0
  GR_GL_FUNCTION_TYPE=__stdcall
  SK_SUPPORT_GPU=0) # TODO change this to 1

if(WIN32)
  target_compile_definitions(skia INTERFACE
    SK_BUILD_FOR_WIN32
    _CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
  target_compile_definitions(skia INTERFACE
    SK_BUILD_FOR_MAC)
else()
  target_compile_definitions(skia INTERFACE
    SK_SAMPLES_FOR_X)
endif()

if(APPLE)
  find_library(COCOA_LIBRARY Cocoa)
  target_link_libraries(skia INTERFACE
    ${COCOA_LIBRARY})
endif()

if(UNIX AND NOT APPLE)
  # Needed for SkFontMgr on Linux
  find_library(FONTCONFIG_LIBRARY fontconfig)
  target_link_libraries(skia INTERFACE
    ${FONTCONFIG_LIBRARY})
endif()
