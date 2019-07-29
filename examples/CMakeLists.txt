# LAF OS
# Copyright (C) 2019  Igara Studio S.A.

if(LAF_BACKEND STREQUAL "skia")
  function(laf_add_example name)
    add_executable(${name} WIN32 ${name}.cpp)
    target_link_libraries(${name} laf-os)
    set_target_properties(${name} PROPERTIES LINK_FLAGS "${LAF_BACKEND_LINK_FLAGS}")
  endfunction()

  laf_add_example(allevents)
  laf_add_example(complextextlayout)
  laf_add_example(helloworld)
endif()
