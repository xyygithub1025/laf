// LAF OS Library
// Copyright (C) 2019-2021  Igara Studio S.A.
// Copyright (C) 2012-2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/osx/window.h"

#include "base/debug.h"
#include "os/event.h"
#include "os/osx/event_queue.h"
#include "os/osx/view.h"
#include "os/osx/window_delegate.h"
#include "os/surface.h"
#include "os/window_spec.h"

@implementation WindowOSXObjc

- (WindowOSXObjc*)initWithImpl:(os::WindowOSX*)impl
                          spec:(const os::WindowSpec*)spec
{
  m_impl = impl;
  m_scale = spec->scale();

  NSScreen* nsScreen;
  if (spec->screen())
    nsScreen = (__bridge NSScreen*)spec->screen()->nativeHandle();
  else
    nsScreen = [NSScreen mainScreen];

  NSWindowStyleMask style = 0;
  if (spec->titled()) style |= NSWindowStyleMaskTitled;
  if (spec->closable()) style |= NSWindowStyleMaskClosable;
  if (spec->minimizable()) style |= NSWindowStyleMaskMiniaturizable;
  if (spec->resizable()) style |= NSWindowStyleMaskResizable;
  if (spec->borderless()) style |= NSWindowStyleMaskBorderless;

  NSRect contentRect;
  if (!spec->contentRect().isEmpty()) {
    contentRect =
      NSMakeRect(spec->contentRect().x,
                 nsScreen.frame.size.height - spec->contentRect().y2(),
                 spec->contentRect().w,
                 spec->contentRect().h);
  }
  else if (!spec->frame().isEmpty()) {
    NSRect frameRect =
      NSMakeRect(spec->frame().x,
                 nsScreen.frame.size.height - spec->frame().y2(),
                 spec->frame().w,
                 spec->frame().h);

    contentRect =
      [NSWindow contentRectForFrameRect:frameRect
                              styleMask:style];
  }
  else {
    // TODO is there a default size for macOS apps?
    contentRect = NSMakeRect(0, 0, 400, 300);
  }

  self = [self initWithContentRect:contentRect
                         styleMask:style
                           backing:NSBackingStoreBuffered
                             defer:NO
                            screen:nsScreen];
  if (!self)
    return nil;

  m_delegate = [[WindowOSXDelegate alloc] initWithWindowImpl:impl];

  // The NSView width and height will be a multiple of 4. In this way
  // all scaled pixels should be exactly the same
  // for Screen Scaling > 1 and <= 4)
  self.contentResizeIncrements = NSMakeSize(4, 4); // TODO make this configurable?

  ViewOSX* view = [[ViewOSX alloc] initWithFrame:contentRect];
  m_view = view;
  [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

  // Redraw the entire window content when we resize it.
  // TODO add support to avoid redrawing the entire window
  self.preservesContentDuringLiveResize = false;

  [self setDelegate:m_delegate];
  [self setContentView:view];

  if (spec->position() == os::WindowSpec::Position::Center) {
    [self center];
  }

  if (spec->parent())
    self.parentWindow = (__bridge NSWindow*)static_cast<os::WindowOSX*>(spec->parent())->nativeHandle();

  [self makeKeyAndOrderFront:self];

  if (spec->floating()) {
    self.level = NSFloatingWindowLevel;
    self.hidesOnDeactivate = true;
  }

  // Hide the "View > Show Tab Bar" menu item
  if ([self respondsToSelector:@selector(setTabbingMode:)])
    [self setTabbingMode:NSWindowTabbingModeDisallowed];

  return self;
}

- (os::WindowOSX*)impl
{
  return m_impl;
}

- (void)removeImpl
{
  [((ViewOSX*)self.contentView) removeImpl];

  [self setDelegate:nil];
  [m_delegate removeImpl];
  m_delegate = nil;

  m_impl = nil;
}

- (int)scale
{
  return m_scale;
}

- (void)setScale:(int)scale
{
  // If the scale is the same, we don't generate a resize event.
  if (m_scale == scale)
    return;

  m_scale = scale;

  if (m_impl) {
    NSRect bounds = [[self contentView] bounds];
    m_impl->onResize(gfx::Size(bounds.size.width,
                               bounds.size.height));
  }
}

- (gfx::Size)clientSize
{
  return gfx::Size([[self contentView] frame].size.width,
                   [[self contentView] frame].size.height);
}

- (gfx::Size)restoredSize
{
  return [self clientSize];
}

- (void)setMousePosition:(const gfx::Point&)position
{
   NSView* view = self.contentView;
   NSPoint pt = NSMakePoint(
     position.x*m_scale,
     view.frame.size.height - position.y*m_scale);

   pt = [view convertPoint:pt toView:view];
   pt = [view convertPoint:pt toView:nil];
   pt = [self convertBaseToScreen:pt];
   pt.y = [[self screen] frame].size.height - pt.y;

   CGPoint pos = CGPointMake(pt.x, pt.y);
   CGEventRef event = CGEventCreateMouseEvent(
     NULL, kCGEventMouseMoved, pos, kCGMouseButtonLeft);
   CGEventPost(kCGHIDEventTap, event);
   CFRelease(event);
}

- (BOOL)setNativeMouseCursor:(os::NativeCursor)cursor
{
  NSCursor* nsCursor = nullptr;

  switch (cursor) {
    case os::NativeCursor::Arrow:
    case os::NativeCursor::Wait:
    case os::NativeCursor::Help:
    case os::NativeCursor::SizeNE:
    case os::NativeCursor::SizeNW:
    case os::NativeCursor::SizeSE:
    case os::NativeCursor::SizeSW:
      nsCursor = [NSCursor arrowCursor];
      break;
    case os::NativeCursor::Crosshair:
      nsCursor = [NSCursor crosshairCursor];
      break;
    case os::NativeCursor::IBeam:
      nsCursor = [NSCursor IBeamCursor];
      break;
    case os::NativeCursor::Link:
      nsCursor = [NSCursor pointingHandCursor];
      break;
    case os::NativeCursor::Forbidden:
      nsCursor = [NSCursor operationNotAllowedCursor];
      break;
    case os::NativeCursor::Move:
      nsCursor = [NSCursor openHandCursor];
      break;
    case os::NativeCursor::SizeNS:
      nsCursor = [NSCursor resizeUpDownCursor];
      break;
    case os::NativeCursor::SizeWE:
      nsCursor = [NSCursor resizeLeftRightCursor];
      break;
    case os::NativeCursor::SizeN:
      nsCursor = [NSCursor resizeUpCursor];
      break;
    case os::NativeCursor::SizeE:
      nsCursor = [NSCursor resizeRightCursor];
      break;
    case os::NativeCursor::SizeS:
      nsCursor = [NSCursor resizeDownCursor];
      break;
    case os::NativeCursor::SizeW:
      nsCursor = [NSCursor resizeLeftCursor];
      break;
  }

  [self.contentView setCursor:nsCursor];
  return (nsCursor ? YES: NO);
}

- (BOOL)setNativeMouseCursor:(const os::Surface*)surface
                       focus:(const gfx::Point&)focus
                       scale:(const int)scale
{
  ASSERT(surface);
  os::SurfaceFormatData format;
  surface->getFormat(&format);
  if (format.bitsPerPixel != 32)
    return NO;

  const int w = scale*surface->width();
  const int h = scale*surface->height();

  if (4*w*h == 0)
    return NO;

  @autoreleasepool {
    NSBitmapImageRep* bmp =
      [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:nil
                      pixelsWide:w
                      pixelsHigh:h
                   bitsPerSample:8
                 samplesPerPixel:4
                        hasAlpha:YES
                        isPlanar:NO
                  colorSpaceName:NSDeviceRGBColorSpace
                    bitmapFormat:NSAlphaNonpremultipliedBitmapFormat
                     bytesPerRow:w*4
                    bitsPerPixel:32];
    if (!bmp)
      return NO;

    uint32_t* dst = (uint32_t*)[bmp bitmapData];
    for (int y=0; y<h; ++y) {
      const uint32_t* src = (const uint32_t*)surface->getData(0, y/scale);
      for (int x=0, u=0; x<w; ++x, ++dst) {
        *dst = *src;
        if (++u == scale) {
          u = 0;
          ++src;
        }
      }
    }

    NSImage* img = [[NSImage alloc] initWithSize:NSMakeSize(w, h)];
    if (!img)
      return NO;

    [img addRepresentation:bmp];

    NSCursor* nsCursor =
      [[NSCursor alloc] initWithImage:img
                              hotSpot:NSMakePoint(scale*focus.x + scale/2,
                                                  scale*focus.y + scale/2)];
    if (!nsCursor)
      return NO;

    [self.contentView setCursor:nsCursor];
    return YES;
  }
}

- (BOOL)canBecomeKeyWindow
{
  if (m_impl)
    return YES;
  else
    return NO;
}

- (void)noResponderFor:(SEL)eventSelector
{
  if (eventSelector == @selector(keyDown:)) {
    // Do nothing (avoid beep)
  }
  else {
    [super noResponderFor:eventSelector];
  }
}

@end

namespace os {

void WindowOSX::createWindow(const os::WindowSpec& spec)
{
  m_nsWindow = [[WindowOSXObjc alloc] initWithImpl:this
                                              spec:&spec];
  m_nsWindow.releasedWhenClosed = true;
}

void WindowOSX::destroyWindow()
{
  if (!m_nsWindow)
    return;

  [m_nsWindow removeImpl];
  [(ViewOSX*)m_nsWindow.contentView destroyMouseTrackingArea];

  // Select other window
  {
    auto app = [NSApplication sharedApplication];
    auto index = [app.windows indexOfObject:m_nsWindow];
    if (index+1 < app.windows.count) {
      ++index;
    }
    else {
      --index;
    }
    if (index >= 0 && index < app.windows.count)
      [[app.windows objectAtIndex:index] makeKeyWindow];
  }

  [m_nsWindow discardEventsMatchingMask:NSEventMaskAny
                            beforeEvent:nullptr];
  [m_nsWindow close];
  m_nsWindow = nil;
}

gfx::Size WindowOSX::clientSize() const
{
  return [m_nsWindow clientSize];
}

gfx::Size WindowOSX::restoredSize() const
{
  return [m_nsWindow restoredSize];
}

gfx::Rect WindowOSX::frame() const
{
  NSRect r = m_nsWindow.frame;
  return gfx::Rect(r.origin.x,
                   m_nsWindow.screen.frame.size.height - r.origin.y - r.size.height,
                   r.size.width, r.size.height);
}

gfx::Rect WindowOSX::contentRect() const
{
  NSRect r = [m_nsWindow contentRectForFrameRect:m_nsWindow.frame];
  return gfx::Rect(r.origin.x,
                   m_nsWindow.screen.frame.size.height - r.origin.y - r.size.height,
                   r.size.width, r.size.height);
}

void WindowOSX::activate()
{
  [m_nsWindow makeKeyWindow];
}

void WindowOSX::maximize()
{
  [m_nsWindow zoom:m_nsWindow];
}

void WindowOSX::minimize()
{
  [m_nsWindow miniaturize:m_nsWindow];
}

bool WindowOSX::isMaximized() const
{
  return false;
}

bool WindowOSX::isMinimized() const
{
  return (m_nsWindow.miniaturized ? true: false);
}

bool WindowOSX::isFullscreen() const
{
  return ((m_nsWindow.styleMask & NSWindowStyleMaskFullScreen) == NSWindowStyleMaskFullScreen);
}

void WindowOSX::setFullscreen(bool state)
{
  if (state) {
    if (!isFullscreen()) {
      // TODO this doesn't work for borderless windows
      [m_nsWindow toggleFullScreen:m_nsWindow];
    }
  }
  else {
    if (isFullscreen()) {
      [m_nsWindow toggleFullScreen:m_nsWindow];
    }
  }
}

std::string WindowOSX::title() const
{
  return [m_nsWindow.title UTF8String];
}

void WindowOSX::setTitle(const std::string& title)
{
  [m_nsWindow setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

void WindowOSX::captureMouse()
{
  // TODO
}

void WindowOSX::releaseMouse()
{
  // TODO
}

void WindowOSX::setMousePosition(const gfx::Point& position)
{
  [m_nsWindow setMousePosition:position];
}

void WindowOSX::performWindowAction(const WindowAction action,
                                    const Event* event)
{
  if (action == WindowAction::Move) {
    // TODO we should use the specified "event"
    [m_nsWindow performWindowDragWithEvent:m_nsWindow.currentEvent];
  }
}

os::ScreenRef WindowOSX::screen() const
{
  ASSERT(m_nsWindow);
  return os::make_ref<os::ScreenOSX>(m_nsWindow.screen);
}

os::ColorSpaceRef WindowOSX::colorSpace() const
{
  if (auto defaultCS = os::instance()->windowsColorSpace())
    return defaultCS;

  ASSERT(m_nsWindow);
  return os::convert_nscolorspace_to_os_colorspace([m_nsWindow colorSpace]);
}

int WindowOSX::scale() const
{
  return [m_nsWindow scale];
}

void WindowOSX::setScale(int scale)
{
  [m_nsWindow setScale:scale];
}

bool WindowOSX::isVisible() const
{
  return m_nsWindow.isVisible;
}

void WindowOSX::setVisible(bool visible)
{
  if (visible) {
    // The main window can be changed only when the NSWindow
    // is visible (i.e. when NSWindow::canBecomeMainWindow
    // returns YES).
    if (m_nsWindow.canBecomeMainWindow)
      [m_nsWindow makeMainWindow];
  }
  else {
    [m_nsWindow setIsVisible:false];
  }
}

bool WindowOSX::setNativeMouseCursor(NativeCursor cursor)
{
  return ([m_nsWindow setNativeMouseCursor:cursor] ? true: false);
}

bool WindowOSX::setNativeMouseCursor(const Surface* surface,
                                     const gfx::Point& focus,
                                     const int scale)
{
  return ([m_nsWindow setNativeMouseCursor:surface
                                     focus:focus
                                     scale:scale] ? true: false);
}

void* WindowOSX::nativeHandle() const
{
  return (__bridge void*)m_nsWindow;
}

} // namespace os
