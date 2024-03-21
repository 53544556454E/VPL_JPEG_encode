#include "mfxutl.h"

namespace mfxUtl
{

mfxU32 Align(mfxU32 value, mfxU32 align)
{
	align = std::bit_ceil(align);

	return (value + (align - 1)) & (~(align - 1));
}

mfxU64 GetFrameSize(mfxU32 four_cc, mfxU32 width, mfxU32 height)
{
	mfxU64 size = 0;
	mfxU64 w = mfxU64(width);
	mfxU64 h = mfxU64(height);

	switch (four_cc)
	{
	case MFX_FOURCC_NV12:
	case MFX_FOURCC_NV21:
		size = w * h; // Y
		size += ((w / 2) * 2) * (h / 2); // UV
		break;

	case MFX_FOURCC_YV12:
	case MFX_FOURCC_IYUV: // alias: MFX_FOURCC_I420
		size = w * h; // Y
		size += w * (h / 2); // U
		size += w * (h / 2); // V
		break;

	case MFX_FOURCC_NV16:
		size = w * h; // Y
		size += ((w / 2) * 2) * h; // UV
		break;

	case MFX_FOURCC_YUY2:
	case MFX_FOURCC_UYVY:
		size = (w * 2) * h;
		break;

	case MFX_FOURCC_RGB565:
	case MFX_FOURCC_R16:
		size = 2 * w * h;
		break;

	case MFX_FOURCC_RGBP:
	case MFX_FOURCC_BGRP:
		size = (w * h) * 3;
		break;

	case MFX_FOURCC_RGB4: // alias: MFX_FOURCC_BGRA
	case MFX_FOURCC_BGR4:
	case MFX_FOURCC_A2RGB10:
	case MFX_FOURCC_AYUV:
	case MFX_FOURCC_AYUV_RGB4:
	case MFX_FOURCC_Y410:
	case MFX_FOURCC_XYUV:
		size = 4 * w * h;
		break;

	case MFX_FOURCC_P8:
	case MFX_FOURCC_P8_TEXTURE:
		size = w * h;
		break;

	case MFX_FOURCC_P010:
	case MFX_FOURCC_P016:
		size = 2 * w * h; // Y
		size += 2 * ((w / 2) * 2) * (h / 2); // UV
		break;

	case MFX_FOURCC_P210:
		size = 2 * w * h;
		size += 2 * ((w / 2) * 2) * h; // UV
		break;

	case MFX_FOURCC_ARGB16:
	case MFX_FOURCC_ABGR16:
	case MFX_FOURCC_Y416:
	case MFX_FOURCC_ABGR16F:
		size = 8 * w * h;
		break;

	case MFX_FOURCC_Y210:
	case MFX_FOURCC_Y216:
		size = 2 * (w * 2) * (h / 2);
		break;

	case MFX_FOURCC_I010:
		size = 2 * w * h; // Y
		size += 2 * w * (h / 2); // U
		size += 2 * w * (h / 2); // V
		break;

	case MFX_FOURCC_I210:
		size = 2 * w * h; // Y
		size += 2 * w * h; // U
		size += 2 * w * h; // V
		break;

	case MFX_FOURCC_I422:
		size = w * h; // Y
		size += w * h; // V
		size += w * h; // U
		break;
	}

	return size;
}

} // namespace mfxUtl
