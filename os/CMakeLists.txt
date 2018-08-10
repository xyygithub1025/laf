# LAF OS
# Copyright (C) 2012-2018  David Capello

set(LAF_OS_SOURCES
  common/freetype_font.cpp
  draw_text.cpp
  system.cpp)

######################################################################
# Skia backend

if(LAF_OS_BACKEND STREQUAL "skia")
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

  include_directories(
    ${SKIA_CONFIG_INCLUDE_DIR}
    ${SKIA_CORE_INCLUDE_DIR}
    ${SKIA_PORTS_INCLUDE_DIR}
    ${SKIA_UTILS_INCLUDE_DIR}
    ${SKIA_CODEC_INCLUDE_DIR}
    ${SKIA_GPU_INCLUDE_DIR}
    ${SKIA_GPU2_INCLUDE_DIR})
  if(WIN32)
    include_directories(${SKIA_ANGLE_INCLUDE_DIR})
  endif()

  set(SKIA_LIBRARIES
    ${SKIA_LIBRARY}
    ${SKIA_OPENGL_LIBRARY}
    CACHE INTERNAL "Skia libraries")

  list(APPEND LAF_OS_SOURCES
    skia/skia_display.cpp
    skia/skia_surface.cpp
    skia/os.cpp)

  if(WIN32)
    list(APPEND LAF_OS_SOURCES
      skia/skia_window_win.cpp
      win/keys.cpp
      win/pen.cpp
      win/winapi.cpp
      win/window.cpp
      win/window_dde.cpp)
  elseif(APPLE)
    list(APPEND LAF_OS_SOURCES
      osx/app.mm
      osx/app_delegate.mm
      osx/event_queue.mm
      osx/keys.mm
      osx/view.mm
      osx/window.mm
      skia/skia_window_osx.mm)
  else()
    list(APPEND LAF_OS_SOURCES
      skia/skia_window_x11.cpp
      x11/event_queue.cpp
      x11/keys.cpp
      x11/window.cpp
      x11/x11.cpp)
  endif()
endif()

######################################################################
# None backend

if(LAF_OS_BACKEND STREQUAL "none")
  list(APPEND LAF_OS_SOURCES
    none/os.cpp)
endif()

######################################################################

if(WIN32)
  list(APPEND LAF_OS_SOURCES
    win/native_dialogs.cpp)
endif()

if(APPLE)
  list(APPEND LAF_OS_SOURCES
    osx/logger.mm
    osx/menus.mm
    osx/native_dialogs.mm)
endif()

if(LAF_OS_WITH_GTK)
  list(APPEND LAF_OS_SOURCES gtk/native_dialogs.cpp)
endif()

add_library(laf-os ${LAF_OS_SOURCES})
target_link_libraries(laf-os laf-ft laf-gfx laf-base)

if(LAF_OS_WITH_GTK)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK gtk+-3.0)
  target_link_directories(laf-os ${GTK_LIBRARY_DIRS})
  target_include_directories(laf-os PRIVATE ${GTK_INCLUDE_DIRS})
  target_compile_definitions(laf-os PRIVATE -DLAF_OS_WITH_GTK)
endif()

if(LAF_OS_BACKEND STREQUAL "skia")
  target_compile_options(laf-os PRIVATE
    -DSK_INTERNAL
    -DSK_GAMMA_SRGB
    -DSK_GAMMA_APPLY_TO_A8
    -DSK_SCALAR_TO_FLOAT_EXCLUDED
    -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1
    -DSK_SUPPORT_OPENCL=0
    -DSK_FORCE_DISTANCE_FIELD_TEXT=0
    -DGR_GL_FUNCTION_TYPE=__stdcall
    -DSK_SUPPORT_GPU=0) # TODO change this to 1
  if(WIN32)
    target_compile_options(laf-os PRIVATE
      -DSK_BUILD_FOR_WIN32)
  elseif(APPLE)
    target_compile_options(laf-os PRIVATE
      -DSK_BUILD_FOR_MAC
      -Wno-ignored-attributes -Wno-unused-result
      # Use Automatic Reference Counting
      -fobjc-arc)
  else()
    target_compile_definitions(laf-os PRIVATE
      -DSK_SAMPLES_FOR_X)
  endif()

  target_link_libraries(laf-os ${SKIA_LIBRARIES})
  if(UNIX AND NOT APPLE)
    target_link_libraries(laf-os fontconfig Xcursor)
  endif()
endif()
