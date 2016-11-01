# LAF
# Copyright (C) 2016  David Capello

# ----------------------------------------------------------------------
# stringencoders library

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)
if(WORDS_BIGENDIAN)
  add_definitions(-DWORDS_BIGENDIAN)
endif()

configure_file(
  stringencoders-config.h.cmakein
  ${CMAKE_CURRENT_BINARY_DIR}/stringencoders/config.h @ONLY)

include_directories(BEFORE stringencoders/src
  ${CMAKE_CURRENT_BINARY_DIR}/stringencoders)

add_executable(modp_b64_gen
  stringencoders/src/arraytoc.c
  stringencoders/src/modp_b64_gen.c)
set_target_properties(modp_b64_gen
  PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stringencoders/modp_b64_data.h
  DEPENDS modp_b64_gen
  COMMAND modp_b64_gen > ${CMAKE_CURRENT_BINARY_DIR}/stringencoders/modp_b64_data.h)

add_library(modpbase64
  stringencoders/src/modp_b64.c
  ${CMAKE_CURRENT_BINARY_DIR}/stringencoders/modp_b64_data.h)

target_include_directories(modpbase64 PUBLIC
  stringencoders/src)

# ----------------------------------------------------------------------
# googletest library

add_subdirectory(googletest)
