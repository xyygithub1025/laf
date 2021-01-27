// LAF OS Library
// Copyright (C) 2020-2021  Igara Studio S.A.
// Copyright (C) 2015  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

class OSXWindowImpl;

@interface OSXWindowDelegate : NSObject {
@private
  OSXWindowImpl* __weak m_impl;
}
@end

@implementation OSXWindowDelegate

- (OSXWindowDelegate*)initWithWindowImpl:(OSXWindowImpl*)impl
{
  m_impl = impl;
  return self;
}

- (void)removeImpl
{
  m_impl = nullptr;
}

- (BOOL)windowShouldClose:(NSWindow*)sender
{
  if (m_impl) {
    os::Event ev;
    ev.setType(os::Event::CloseDisplay);
    m_impl->queueEvent(ev);
  }
  return NO;
}

- (void)windowWillClose:(NSNotification*)notification
{
  if (m_impl)
    m_impl->onClose();
}

- (void)windowWillStartLiveResize:(NSNotification*)notification
{
  if (m_impl)
    m_impl->onStartResizing();
}

- (NSSize)windowWillResize:(NSWindow*)sender
                    toSize:(NSSize)frameSize
{
  NSView* view = sender.contentView;
  gfx::Size sz(view.bounds.size.width, view.bounds.size.height);
  if (m_impl)
    m_impl->onResizing(sz);
  return frameSize;
}

- (void)windowDidEndLiveResize:(NSNotification*)notification
{
  if (m_impl)
    m_impl->onEndResizing();
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
}

- (void)windowWillEnterFullScreen:(NSNotification*)notification
{
  if (m_impl)
    m_impl->onStartResizing();
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification
{
  if (m_impl)
    m_impl->onEndResizing();
}

- (void)windowWillExitFullScreen:(NSNotification*)notification
{
  if (m_impl)
    m_impl->onStartResizing();
}

- (void)windowDidExitFullScreen:(NSNotification*)notification
{
  // After exiting the full screen mode we have to re-create the skia
  // surface and re-draw the entire screen. Without this there will be
  // some cases where the app view is not updated anymore until we
  // resize the window.
  if (m_impl)
    m_impl->onEndResizing();
}

@end
