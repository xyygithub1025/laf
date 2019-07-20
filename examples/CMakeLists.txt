# LAF OS
# Copyright (C) 2019  Igara Studio S.A.

if(LAF_BACKEND STREQUAL "skia")
  add_executable(helloworld WIN32 helloworld.cpp)
  target_link_libraries(helloworld laf-os)
  set_target_properties(helloworld PROPERTIES LINK_FLAGS "${LAF_BACKEND_LINK_FLAGS}")
endif()
