// LAF OS Library
// Copyright (C) 2018  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/skia/skia_color_space.h"

#include "SkString.h"

#include <algorithm>

// Defined in skia/src/core/SkICC.cpp
extern SkString SkICCGetColorProfileTag(const SkColorSpaceTransferFn& fn,
                                        const SkMatrix44& toXYZD50);

namespace os {

// Copied from skia/src/core/SkColorSpacePriv.h
static constexpr float gSRGB_toXYZD50[] {
  0.4360747f, 0.3850649f, 0.1430804f, // Rx, Gx, Bx
  0.2225045f, 0.7168786f, 0.0606169f, // Ry, Gy, By
  0.0139322f, 0.0971045f, 0.7141733f, // Rz, Gz, Bz
};

SkiaColorSpace::SkiaColorSpace(const gfx::ColorSpacePtr& gfxcs)
  : m_gfxcs(gfxcs),
    m_skcs(nullptr)
{
  switch (m_gfxcs->type()) {

    case gfx::ColorSpace::None:
      if (m_gfxcs->name().empty())
        m_gfxcs->setName("None");
      break;

    case gfx::ColorSpace::sRGB:
    case gfx::ColorSpace::RGB:
      if (gfxcs->hasGamma()) {
        if (gfxcs->gamma() == 1.0)
          m_skcs = SkColorSpace::MakeSRGBLinear();
        else {
          SkColorSpaceTransferFn fn;
          fn.fA = 1.0f;
          fn.fB = fn.fC = fn.fD = fn.fE = fn.fF = 0.0f;
          fn.fG = gfxcs->gamma();
          m_skcs = SkColorSpace::MakeRGB(fn, SkColorSpace::kSRGB_Gamut);
        }
      }
      else {
        SkColorSpaceTransferFn skFn;
        SkMatrix44 toXYZD50(SkMatrix44::kUninitialized_Constructor);

        if (m_gfxcs->hasPrimaries()) {
          const gfx::ColorSpacePrimaries* primaries = m_gfxcs->primaries();
          SkColorSpacePrimaries skPrimaries;
          skPrimaries.fRX = primaries->rx;
          skPrimaries.fRY = primaries->ry;
          skPrimaries.fGX = primaries->gx;
          skPrimaries.fGY = primaries->gy;
          skPrimaries.fBX = primaries->bx;
          skPrimaries.fBY = primaries->by;
          skPrimaries.fWX = primaries->wx;
          skPrimaries.fWY = primaries->wy;

          if (!skPrimaries.toXYZD50(&toXYZD50))
            toXYZD50.set3x3RowMajorf(gSRGB_toXYZD50);
        }

        if (m_gfxcs->hasTransferFn()) {
          const gfx::ColorSpaceTransferFn* fn = m_gfxcs->transferFn();
          skFn.fG = fn->g;
          skFn.fA = fn->a;
          skFn.fB = fn->b;
          skFn.fC = fn->c;
          skFn.fD = fn->d;
          skFn.fE = fn->e;
          skFn.fF = fn->f;
        }

        if (m_gfxcs->hasTransferFn()) {
          if (!m_gfxcs->hasPrimaries())
            toXYZD50.set3x3RowMajorf(gSRGB_toXYZD50);
          m_skcs = SkColorSpace::MakeRGB(skFn, toXYZD50);
        }
        else if (m_gfxcs->hasPrimaries()) {
          m_skcs = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma, toXYZD50);
        }
        else {
          m_skcs = SkColorSpace::MakeSRGB();
        }
      }
      break;

    case gfx::ColorSpace::ICC: {
      m_skcs = SkColorSpace::MakeICC(m_gfxcs->iccData(),
                                     m_gfxcs->iccSize());
      break;
    }
  }

  // TODO read color profile name from ICC data

  if (m_skcs && m_gfxcs->name().empty()) {
    SkColorSpaceTransferFn fn;
    SkMatrix44 toXYZD50(SkMatrix44::kIdentity_Constructor);
    if (m_skcs->isNumericalTransferFn(&fn) &&
        m_skcs->toXYZD50(&toXYZD50)) {
      SkString desc = SkICCGetColorProfileTag(fn, toXYZD50);
      m_gfxcs->setName(desc.c_str());
    }
  }
}

SkiaColorSpaceConversion::SkiaColorSpaceConversion(
  const os::ColorSpacePtr& srcColorSpace,
  const os::ColorSpacePtr& dstColorSpace)
  : m_srcCS(srcColorSpace),
    m_dstCS(dstColorSpace)
{
  ASSERT(srcColorSpace);
  ASSERT(dstColorSpace);
  m_xform =
    SkColorSpaceXform::New(
      static_cast<const SkiaColorSpace*>(srcColorSpace.get())->skColorSpace().get(),
      static_cast<const SkiaColorSpace*>(dstColorSpace.get())->skColorSpace().get());
}

bool SkiaColorSpaceConversion::convert(uint32_t* dst, const uint32_t* src, int n)
{
  return m_xform->apply(SkColorSpaceXform::kRGBA_8888_ColorFormat, dst,
                        SkColorSpaceXform::kRGBA_8888_ColorFormat, src, n,
                        kUnpremul_SkAlphaType);
}

} // namespace os
