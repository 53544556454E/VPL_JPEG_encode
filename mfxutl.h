#pragma once

#include <bit>

#include <vpl/mfxdefs.h>
#include <vpl/mfxstructures.h>

namespace mfxUtl
{
mfxU32 Align(mfxU32 value, mfxU32 align);
mfxU64 GetFrameSize(mfxU32 four_cc, mfxU32 width, mfxU32 height);
}
