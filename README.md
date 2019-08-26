# LAF: The Lost Application Framework

[![Build Status](https://travis-ci.org/aseprite/laf.svg)](https://travis-ci.org/aseprite/laf)
[![Build status](https://ci.appveyor.com/api/projects/status/3hqecxtkobx2gofo?svg=true)](https://ci.appveyor.com/project/dacap/laf)
[![MIT Licensed](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.txt)

A library to create Windows, macOS, and Linux desktop applications.

This library is under active development so we don't provide API or
ABI compatibility at this moment.

## Modules

* [base](base): Base functions for any kind of application.
* [gfx](gfx): Generic graphics classes (rectangle, point, region, etc.)
* [ft](ft): FreeType wrapper used by the `os` module (requires `freetype` library as dependency)
* [os](os): Functions to create windows in your Operating System desktop.

## Dependencies

`laf` requires:

* A compiled version of Skia branch m76 (you can check the
  [instructions to compile Skia on Aseprite](https://github.com/aseprite/aseprite/blob/master/INSTALL.md#building-skia-dependency))
* `freetype` and `harfbuzz` to draw text.
* `pixman` in case that you are going to use `gfx::Region` without the
  `laf-os` (improvable case, generally if you're using `laf-os` you
  will use Skia, so there is no need for `pixman`)

## Compile

To compile with Skia as backend you have to specify a valid compiled version of Skia:

```
cd /path-with-laf-source-code
mkdir build
cd build
cmake -G Ninja -DSKIA_DIR=/path-to-skia-source-code -DSKIA_OUT_DIR=/path-to-skia-source-code/out/Release /path-with-laf-source-code
ninja
./examples/helloworld
```

To compile only the library (without examples and tests) you can
disable the `LAF_WITH_EXAMPLES`/`LAF_WITH_TESTS` options:

```
cmake -DLAF_WITH_EXAMPLES=OFF -DLAF_WITH_TESTS=OFF ...
```

## Running Tests

You can use `ctest` to run all tests:

```
cd build
ninja
ctest
```

## License

LAF is distributed under the terms of [the MIT license](LICENSE.txt).

Some functions in LAF depends on third party libraries (you should
include these license notices when you distribute your software):

* [base::encode/decode_base64](base/base64.cpp) functions use
  [stringencoders](https://github.com/client9/stringencoders) by
  [Nick Galbreath](https://github.com/client9)
  ([MIT license](https://github.com/aseprite/stringencoders/blob/master/LICENSE)).
* Tests use the [Google Test](https://github.com/aseprite/googletest/tree/master/googletest)
  framework by Google Inc. licensed under
  [a BSD-like license](https://github.com/aseprite/googletest/blob/master/googletest/LICENSE).
* Color spaces, `gfx::Region`, and the `laf::os` library use code from
  the [Skia library](https://skia.org) by Google Inc. licensed under
  [a BSD-like license](https://github.com/aseprite/skia/blob/master/LICENSE).
* `gfx::Region` uses the pixman library if you are not compiling with
  the Skia backend (e.g. a if you create a Command Line utility that
  uses the `gfx::Region` class), which is distributed under the [MIT
  License](https://cgit.freedesktop.org/pixman/tree/COPYING).
