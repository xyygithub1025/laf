// LAF OS Library
// Copyright (C) 2020-2022  Igara Studio S.A.
// Copyright (C) 2012-2018  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <Cocoa/Cocoa.h>
#include <vector>

#include "base/fs.h"
#include "os/common/file_dialog.h"
#include "os/keys.h"
#include "os/native_cursor.h"
#include "os/osx/native_dialogs.h"
#include "os/window.h"

#include <map>

namespace {

// macOS does a super strange thing to handle the keyboard shortcuts
// when a NSSavePanel is open: it sends all the Command+key
// combinations to the main menu to handle the most basic shortcuts
// like Command+C (to run the copy: selector), or Command+V (paste:),
// etc.
//
// So what we have to do if the main menu doesn't provide the standard
// Edit selectors? (which is our case with our MenuItemOSX and MenuOSX
// implementations):
//
// 1. Before we open the NSSavePanel, find the Edit menu that was
//    specified by the user with os::MenuItem::setAsStandardEditMenuItem()
// 2. Replace the Edit menu with a custom made one with the
//    standard Edit menu prepared especially with selectors
//    (undo:, redo:, cut:, etc.)
// 3. Each menu item that contains a standard keyboard shortcut
//    (Command+C, Command+V, etc.) must be modified, because those
//    shortcuts are now used by this new Edit menu. So we remove the
//    keyEquivalent of each one. This is necessary only when one of
//    those items are outside the replaced Edit menu (e.g. if we have
//    Command+A to select all in other menu like Select > All, instead
//    of Edit > Select All)
// 4. After the NSSavePanel is closed, we restore all keyEquivalent
//    shortcuts and the old Edit menu.
//
class OSXEditMenuHack {
public:
  OSXEditMenuHack(NSMenuItem* currentEditMenuItem)
    : m_editMenuItem(currentEditMenuItem) {
    if (!m_editMenuItem)
      return;

    auto editMenu = [NSMenu new];

    // Check MenuItemOSX::syncTitle(), but basically on macOS the
    // NSMenu title is the one displayed for NSMenuItem with submenus.
    editMenu.title = currentEditMenuItem.title;

    [editMenu addItem:newItem(@"Undo", @selector(undo:), @"z", NSEventModifierFlagCommand)];
    [editMenu addItem:newItem(@"Redo", @selector(redo:), @"Z", NSEventModifierFlagCommand)];
    [editMenu addItem:[NSMenuItem separatorItem]];
    [editMenu addItem:newItem(@"Cut", @selector(cut:), @"x", NSEventModifierFlagCommand)];
    [editMenu addItem:newItem(@"Copy", @selector(copy:), @"c", NSEventModifierFlagCommand)];
    [editMenu addItem:newItem(@"Paste", @selector(paste:), @"v", NSEventModifierFlagCommand)];
    [editMenu addItem:newItem(@"Delete", @selector(delete:), @"", 0)];
    [editMenu addItem:newItem(@"Select All", @selector(selectAll:), @"a", NSEventModifierFlagCommand)];

    m_submenu = m_editMenuItem.submenu;
    m_editMenuItem.submenu = editMenu;
  }

  ~OSXEditMenuHack() {
    if (!m_editMenuItem)
      return;

    // Restore all key equivalents
    for (auto& kv : m_restoreKeys)
      kv.first.keyEquivalent = kv.second;

    m_editMenuItem.submenu = m_submenu;
  }

private:
  NSMenuItem* newItem(NSString* title, SEL sel,
                      NSString* key,
                      NSEventModifierFlags flags) {
    auto item = [[NSMenuItem alloc] initWithTitle:title
                 action:sel
                 keyEquivalent:key];
    if (flags)
      item.keyEquivalentModifierMask = flags;

    // Search for menu items in the main menu that already contain the
    // same keyEquivalent, and disable them temporarily.
    disableMenuItemsWithKeyEquivalent(
      [[NSApplication sharedApplication] mainMenu],
      key, flags);

    return item;
  }

  void disableMenuItemsWithKeyEquivalent(NSMenu* menu,
                                         NSString* key,
                                         NSEventModifierFlags flags) {
    for (NSMenuItem* item in menu.itemArray) {
      if ([item.keyEquivalent isEqualToString:key] &&
          item.keyEquivalentModifierMask == flags) {
        m_restoreKeys[item] = item.keyEquivalent;
        item.keyEquivalent = @"";
      }
      if (item.submenu)
        disableMenuItemsWithKeyEquivalent(item.submenu, key, flags);
    }
  }

  NSMenuItem* m_editMenuItem = nullptr;
  NSMenu* m_submenu = nullptr;
  std::map<NSMenuItem*, NSString*> m_restoreKeys;
};

} // anonymous namespace

namespace os {
extern NSMenuItem* g_standardEditMenuItem;
}

@interface OpenSaveHelper : NSObject {
@private
  NSSavePanel* panel;
  os::Window* window;
  int result;
}
- (id)init;
- (void)setPanel:(NSSavePanel*)panel;
- (void)setWindow:(os::Window*)window;
- (void)runModal;
- (int)result;
@end

@implementation OpenSaveHelper

- (id)init
{
  if (self = [super init]) {
    result = NSFileHandlingPanelCancelButton;
  }
  return self;
}

- (void)setPanel:(NSSavePanel*)newPanel
{
  panel = newPanel;
}

- (void)setWindow:(os::Window*)newWindow
{
  window = newWindow;
}

// This is executed in the main thread.
- (void)runModal
{
  os::NativeCursor oldCursor = window->nativeCursor();
  window->setCursor(os::NativeCursor::Arrow);

  OSXEditMenuHack hack(os::g_standardEditMenuItem);
  [[[NSApplication sharedApplication] mainMenu] setAutoenablesItems:NO];

#ifndef __MAC_10_6              // runModalForTypes is deprecated in 10.6
  if ([panel isKindOfClass:[NSOpenPanel class]]) {
    // As we're using OS X 10.4 framework, it looks like runModal
    // doesn't recognize the allowedFileTypes property. So we force it
    // using runModalForTypes: selector.

    result = [(NSOpenPanel*)panel runModalForTypes:[panel allowedFileTypes]];
  }
  else
#endif
  {
    result = [panel runModal];
  }

  window->setCursor(oldCursor);
  NSWindow* nsWindow = (__bridge NSWindow *)window->nativeHandle();
  [nsWindow makeKeyAndOrderFront:nil];
  [[[NSApplication sharedApplication] mainMenu] setAutoenablesItems:YES];
}

- (int)result
{
  return result;
}

@end

namespace os {

class FileDialogOSX : public CommonFileDialog {
public:
  FileDialogOSX() {
  }

  std::string fileName() override {
    return m_filename;
  }

  void getMultipleFileNames(base::paths& output) override {
    output = m_filenames;
  }

  void setFileName(const std::string& filename) override {
    m_filename = filename;
  }

  bool show(Window* window) override {
    bool retValue = false;
    @autoreleasepool {
      NSSavePanel* panel = nil;

      if (m_type == Type::SaveFile) {
        panel = [NSSavePanel new];
      }
      else {
        panel = [NSOpenPanel new];
        [(NSOpenPanel*)panel setAllowsMultipleSelection:(m_type == Type::OpenFiles ? YES: NO)];
        [(NSOpenPanel*)panel setCanChooseFiles:(m_type != Type::OpenFolder ? YES: NO)];
        [(NSOpenPanel*)panel setCanChooseDirectories:(m_type == Type::OpenFolder ? YES: NO)];
      }

      [panel setTitle:[NSString stringWithUTF8String:m_title.c_str()]];
      [panel setCanCreateDirectories:YES];

      if (m_type != Type::OpenFolder && !m_filters.empty()) {
        NSMutableArray* types = [[NSMutableArray alloc] init];
        // The first extension in the array is used as the default one.
        if (!m_defExtension.empty())
          [types addObject:[NSString stringWithUTF8String:m_defExtension.c_str()]];
        for (const auto& filter : m_filters)
          [types addObject:[NSString stringWithUTF8String:filter.first.c_str()]];
        [panel setAllowedFileTypes:types];
        if (m_type == Type::SaveFile)
          [panel setAllowsOtherFileTypes:NO];
      }

      // Always show the extension
      [panel setExtensionHidden:NO];

      std::string defPath = base::get_file_path(m_filename);
      std::string defName = base::get_file_name(m_filename);
      if (!defPath.empty())
        [panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:defPath.c_str()]]];
      if (!defName.empty())
        [panel setNameFieldStringValue:[NSString stringWithUTF8String:defName.c_str()]];

      OpenSaveHelper* helper = [OpenSaveHelper new];
      [helper setPanel:panel];
      [helper setWindow:window];
      [helper performSelectorOnMainThread:@selector(runModal) withObject:nil waitUntilDone:YES];

      if ([helper result] == NSFileHandlingPanelOKButton) {
        if (m_type == Type::OpenFiles) {
          for (NSURL* url in [(NSOpenPanel*)panel URLs]) {
            m_filename = [[url path] UTF8String];
            m_filenames.push_back(m_filename);
          }
        }
        else {
          NSURL* url = [panel URL];
          m_filename = [[url path] UTF8String];
          m_filenames.push_back(m_filename);
        }
        retValue = true;
      }
    }
    return retValue;
  }

private:

  std::string m_filename;
  base::paths m_filenames;
};

NativeDialogsOSX::NativeDialogsOSX()
{
}

FileDialogRef NativeDialogsOSX::makeFileDialog()
{
  return make_ref<FileDialogOSX>();
}

} // namespace os
