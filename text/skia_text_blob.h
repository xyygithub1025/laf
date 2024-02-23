// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_SKIA_TEXT_BLOB_INCLUDED
#define LAF_TEXT_SKIA_TEXT_BLOB_INCLUDED
#pragma once

#include "text/text_blob.h"

#include "include/core/SkTextBlob.h"

namespace text {

class SkiaTextBlob : public TextBlob {
public:
  SkiaTextBlob(sk_sp<SkTextBlob> skTextBlob);

  sk_sp<SkTextBlob> skTextBlob() const { return m_skTextBlob; }

private:
  sk_sp<SkTextBlob> m_skTextBlob;
};

} // namespace text

#endif
