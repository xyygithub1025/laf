// LAF OS Library
// Copyright (C) 2020  Igara Studio S.A.
// Copyright (C) 2016-2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_WIN_WINTAB_H_INCLUDED
#define OS_WIN_WINTAB_H_INCLUDED
#pragma once

#include "base/dll.h"

#include <windows.h>
#include "wacom/wintab.h"

#define PACKETDATA (PK_CURSOR | PK_BUTTONS | PK_X | PK_Y | PK_NORMAL_PRESSURE)
#define PACKETMODE (PK_BUTTONS)
#include "wacom/pktdef.h"

namespace os {

  // Wintab API wrapper
  // Read http://www.wacomeng.com/windows/docs/Wintab_v140.htm for more information.
  class WintabAPI {
  public:
    WintabAPI();
    ~WintabAPI();

    HCTX open(HWND hwnd);
    void close(HCTX ctx);
    bool packet(HCTX ctx, UINT serial, LPVOID packet);
    void overlap(HCTX ctx, BOOL state);

    LONG minPressure() const { return m_minPressure; }
    LONG maxPressure() const { return m_maxPressure; }

  private:
    bool loadWintab();
    bool checkDll();

    base::dll m_wintabLib;
    LONG m_minPressure = 0;
    LONG m_maxPressure = 1000;
  };

} // namespace os

#endif
