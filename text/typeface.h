// LAF Text Library
// Copyright (c) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_TYPEFACE_H_INCLUDED
#define LAF_TEXT_TYPEFACE_H_INCLUDED
#pragma once

#include "base/ref.h"

namespace text {

  class Typeface : public base::RefCount {
  protected:
    virtual ~Typeface() { }
  public:
    virtual FontStyle fontStyle() const = 0;
  };

} // namespace text

#endif
