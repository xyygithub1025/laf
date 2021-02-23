// LAF OS Library
// Copyright (c) 2020-2021  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/osx/system.h"

#include "os/osx/screen.h"

namespace os {

gfx::Color SystemOSX::getColorFromScreen(const gfx::Point& screenPosition) const
{
  gfx::Color color = gfx::ColorNone;
  CGImageRef image = CGDisplayCreateImageForRect(CGMainDisplayID(),
                                                 CGRectMake(screenPosition.x, screenPosition.y, 1, 1));
  if (image) {
    CGBitmapInfo info = CGImageGetBitmapInfo(image);
    CGDataProviderRef provider = CGImageGetDataProvider(image);
    if (provider) {
      NSData* data = (__bridge NSData*)CGDataProviderCopyData(provider);
      const uint8_t* bytes = (const uint8_t*)data.bytes;

      // TODO support other formats
      const int bpp = CGImageGetBitsPerPixel(image);
      if (bpp == 32) {
        // TODO kCGBitmapByteOrder32Big
        if (info & kCGImageAlphaLast) {
          color = gfx::rgba(bytes[2], bytes[1], bytes[0], bytes[3]);
        }
        else {
          color = gfx::rgba(bytes[3], bytes[2], bytes[1], bytes[0]);
        }
      }

      // If we release the provider then CGImageRelease() crashes
      //CGDataProviderRelease(provider);
    }
    CGImageRelease(image);
  }
  return color;
}

ScreenRef SystemOSX::mainScreen()
{
  return make_ref<ScreenOSX>([NSScreen mainScreen]);
}

void SystemOSX::listScreens(ScreenList& list)
{
  auto screens = [NSScreen screens];
  for (NSScreen* screen : screens)
    list.push_back(make_ref<ScreenOSX>(screen));
}

}
