// LAF Base Library
// Copyright (c) 2001-2016 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_BASE_H_INCLUDED
#define BASE_BASE_H_INCLUDED
#pragma once

#include "base/config.h"

#include <math.h>

#undef NULL
#ifdef __cplusplus
  #define NULL nullptr
#else
  #define NULL ((void*)0)
#endif

#undef MIN
#undef MAX
#undef MID
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   ((x) > (y) ? ((y) > (z) ? (y) : ((x) > (z) ?    \
                       (z) : (x))) : ((y) > (z) ? ((z) > (x) ? (z) : \
                       (x)): (y)))

#undef CLAMP
#define CLAMP(x,y,z) MAX((x), MIN((y), (z)))

#undef ABS
#undef SGN
#define ABS(x)       (((x) >= 0) ? (x) : (-(x)))
#define SGN(x)       (((x) >= 0) ? 1 : -1)



//////////////////////////////////////////////////////////////////////
// Overloaded new/delete operators to detect memory-leaks

#if defined __cplusplus && defined LAF_MEMLEAK

#include <new>

#ifdef _NOEXCEPT
  #define LAF_NOEXCEPT _NOEXCEPT
#else
  #define LAF_NOEXCEPT
#endif

void* operator new(std::size_t size);
void* operator new[](std::size_t size);
void operator delete(void* ptr) LAF_NOEXCEPT;
void operator delete[](void* ptr) LAF_NOEXCEPT;

#endif

#endif
