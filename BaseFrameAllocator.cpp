#include "BaseFrameAllocator.h"

BaseFrameAllocator::BaseFrameAllocator():
	mfxFrameAllocator()
{
	pthis = this;
	Alloc = BaseAlloc;
	Lock = BaseLock;
	Unlock = BaseUnlock;
	GetHDL = BaseGetHDL;
	Free = BaseFree;
}

BaseFrameAllocator::~BaseFrameAllocator()
{
}

mfxStatus BaseFrameAllocator::UpdateLockedFrame(mfxFrameData *ptr, mfxU32 four_cc, mfxU8 *frame_base, mfxU32 width, mfxU32 height)
{
	if (!ptr || !frame_base)
		return MFX_ERR_NULL_PTR;

	auto SetPitch = [&ptr](mfxU32 pitch)
		{
			ptr->PitchHigh = static_cast<mfxU16>(pitch >> 16);
			ptr->PitchLow = static_cast<mfxU16>(pitch & 0xffff);
		};

	switch (four_cc)
	{
	case MFX_FOURCC_NV12: // 4:2:0, semi plainer, Y-UV
	case MFX_FOURCC_NV16: // 4:2:2, semi plainer, Y-UV
		ptr->Y = frame_base;
		ptr->UV = ptr->Y + width * height;
		ptr->V = nullptr;
		ptr->A = nullptr;
		SetPitch(width);
		break;

	case MFX_FOURCC_YV12: // 4:2:0, fully plainer, Y-V-U
		ptr->Y = frame_base;
		ptr->V = ptr->Y + width * height;
		ptr->U = ptr->V + width * (height / 2);
		ptr->A = nullptr;
		SetPitch(width);
		break;

	case MFX_FOURCC_YUY2: // 4:2:2, packed, Y-U-Y-V
		ptr->Y = frame_base;
		ptr->U = ptr->Y + 1;
		ptr->V = ptr->Y + 3;
		ptr->A = nullptr;
		SetPitch(2 * width);
		break;

	case MFX_FOURCC_RGB565: // 4:4:4, packed, B-G-R
		ptr->B = frame_base;
		ptr->G = ptr->B;
		ptr->R = ptr->B;
		ptr->A = nullptr;
		SetPitch(2 * width);
		break;

	case MFX_FOURCC_RGBP: // 4:4:4, fully plainer, R-G-B
		ptr->R = frame_base;
		ptr->G = ptr->R + width * height;
		ptr->B = ptr->G + width * height;
		ptr->A = nullptr;
		SetPitch(width);
		break;

	case MFX_FOURCC_RGB4: // 4:4:4, packed, B-G-R-A, alias MFX_FOURCC_BGRA
	case MFX_FOURCC_AYUV_RGB4: // 4:4:4, packed, B-G-R-A
		ptr->B = frame_base;
		ptr->G = ptr->B + 1;
		ptr->R = ptr->B + 2;
		ptr->A = ptr->B + 3;
		SetPitch(4 * width);
		break;

	case MFX_FOURCC_P8:
	case MFX_FOURCC_P8_TEXTURE:
		ptr->Y = frame_base;
		ptr->U = nullptr;
		ptr->V = nullptr;
		ptr->A = nullptr;
		SetPitch(width);
		break;

	case MFX_FOURCC_P010: // 4:2:0, semi plainer, Y-UV
	case MFX_FOURCC_P016: // 4:2:0, semi plainer, Y-UV
	case MFX_FOURCC_P210: // 4:2:2, semi plainer, Y-UV
		ptr->Y = frame_base;
		ptr->UV = ptr->Y + width * height * 2;
		ptr->V = nullptr;
		ptr->A = nullptr;
		SetPitch(2 * width);
		break;

	case MFX_FOURCC_BGR4:// 4:4:4, packed, R-G-B-A
		ptr->R = frame_base;
		ptr->G = ptr->R + 1;
		ptr->B = ptr->R + 2;
		ptr->A = ptr->R + 3;
		SetPitch(4 * width);
		break;

	case MFX_FOURCC_A2RGB10: // 4:4:4, packed, B-G-R-A
		ptr->R = nullptr;
		ptr->G = nullptr;
		ptr->A2RGB10 = reinterpret_cast<mfxA2RGB10 *>(frame_base);
		ptr->A = nullptr;
		SetPitch(4 * width);
		break;

	case MFX_FOURCC_ARGB16: // 4:4:4, packed, B-G-R-A
		ptr->B = frame_base;
		ptr->G = ptr->B + 2;
		ptr->R = ptr->B + 4;
		ptr->A = ptr->B + 6;
		SetPitch(8 * width);
		break;

	case MFX_FOURCC_ABGR16: // 4:4:4, packed, R-G-B-A
		ptr->R = frame_base;
		ptr->G = ptr->R + 2;
		ptr->B = ptr->R + 4;
		ptr->A = ptr->R + 6;
		SetPitch(8 * width);
		break;

	case MFX_FOURCC_R16:
		ptr->R = frame_base;
		ptr->G = nullptr;
		ptr->B = nullptr;
		ptr->A = nullptr;
		SetPitch(2 * width);
		break;

	case MFX_FOURCC_AYUV: // 4:4:4, packed, V-U-Y-A
	case MFX_FOURCC_XYUV: // 4:4:4, packed, V-U-Y-X
		ptr->V = frame_base;
		ptr->U = ptr->V + 1;
		ptr->Y = ptr->V + 2;
		ptr->A = ptr->V + 3;
		SetPitch(4 * width);
		break;

	case MFX_FOURCC_UYVY: // 4:2:2, packed, U-Y-V-Y
		ptr->U = frame_base;
		ptr->Y = ptr->U + 1;
		ptr->V = ptr->U + 2;
		ptr->A = nullptr;
		SetPitch(2 * width);
		break;

	case MFX_FOURCC_Y210: // 4:2:2, packed, Y-U-Y-V
	case MFX_FOURCC_Y216: // 4:2:2, packed, Y-U-Y-V
		ptr->Y16 = reinterpret_cast<mfxU16 *>(frame_base);
		ptr->U16 = ptr->Y16 + 1;
		ptr->V16 = ptr->Y16 + 3;
		ptr->A = nullptr;
		SetPitch(4 * width);
		break;

	case MFX_FOURCC_Y410: // 4:4:4, packed, U-Y-V-A
		ptr->U = frame_base;
		ptr->Y = ptr->U;
		ptr->V = ptr->U;
		ptr->A = ptr->U;
		SetPitch(4 * width);
		break;

	case MFX_FOURCC_Y416: // 4:4:4, packed, U-Y-V-A
		ptr->U16 = reinterpret_cast<mfxU16 *>(frame_base);
		ptr->Y16 = ptr->U16 + 1;
		ptr->V16 = ptr->U16 + 2;
		ptr->A = reinterpret_cast<mfxU8 *>(ptr->U16 + 3);
		SetPitch(8 * width);
		break;

	case MFX_FOURCC_NV21: // 4:2:0, semi plainer, Y-VU
		ptr->Y = frame_base;
		ptr->VU = ptr->Y + width * height;
		ptr->V = nullptr;
		ptr->A = nullptr;
		SetPitch(width);
		break;

	case MFX_FOURCC_IYUV: // 4:2:0, fully plainer, Y-U-V, alias MFX_FOURCC_I420
		ptr->Y = frame_base;
		ptr->U = ptr->Y + width * height;
		ptr->V = ptr->U + width * (height / 2);
		ptr->A = nullptr;
		SetPitch(width);
		break;

	case MFX_FOURCC_I010: // 4:2:0, fully plainer, Y-U-V
		ptr->Y16 = reinterpret_cast<mfxU16 *>(frame_base);
		ptr->U16 = ptr->Y16 + width * height;
		ptr->V16 = ptr->U16 + width * (height / 2);
		ptr->A = nullptr;
		SetPitch(2 * width);
		break;

	case MFX_FOURCC_I210: // 4:2:2, fully plainer, Y-U-V
		ptr->Y16 = reinterpret_cast<mfxU16 *>(frame_base);
		ptr->U16 = ptr->Y16 + width * height;
		ptr->V16 = ptr->U16 + width * height;
		ptr->A = nullptr;
		SetPitch(2 * width);
		break;

	case MFX_FOURCC_I422: // 4:2:2, fully plainer, Y-U-V
		ptr->Y = frame_base;
		ptr->U = ptr->Y + width * height;
		ptr->V = ptr->U + width * height;
		ptr->A = nullptr;
		SetPitch(width);
		break;

	case MFX_FOURCC_BGRP: // 4:4:4, fully plainer, B-G-R
		ptr->B = frame_base;
		ptr->G = ptr->B + width * height;
		ptr->R = ptr->G + width * height;
		ptr->A = nullptr;
		SetPitch(width);
		break;

	case MFX_FOURCC_ABGR16F: // 4:4:4, packed, R-G-B-A
		ptr->Y = nullptr;
		ptr->U = nullptr;
		ptr->ABGRFP16 = reinterpret_cast<mfxABGR16FP *>(frame_base);
		ptr->A = nullptr;
		SetPitch(8 * width);
		break;

	default:
		return MFX_ERR_UNSUPPORTED;
	}

	return MFX_ERR_NONE;
}

void BaseFrameAllocator::UpdateUnlockedFrame(mfxFrameData *ptr)
{
	if (ptr)
	{
		ptr->PitchHigh = 0;
		ptr->PitchLow = 0;
		ptr->R = nullptr;
		ptr->G = nullptr;
		ptr->B = nullptr;
		ptr->A = nullptr;
	}
}

mfxStatus MFX_CDECL BaseFrameAllocator::BaseAlloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
	if (!pthis)
		return MFX_ERR_INVALID_HANDLE;

	BaseFrameAllocator *self = reinterpret_cast<BaseFrameAllocator *>(pthis);

	return self->AllocFrames(request, response);
}

mfxStatus MFX_CDECL BaseFrameAllocator::BaseLock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
	if (!pthis)
		return MFX_ERR_INVALID_HANDLE;

	BaseFrameAllocator *self = reinterpret_cast<BaseFrameAllocator *>(pthis);

	return self->LockFrame(mid, ptr);
}

mfxStatus MFX_CDECL BaseFrameAllocator::BaseUnlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
	if (!pthis)
		return MFX_ERR_INVALID_HANDLE;

	BaseFrameAllocator *self = reinterpret_cast<BaseFrameAllocator *>(pthis);

	return self->UnlockFrame(mid, ptr);
}

mfxStatus MFX_CDECL BaseFrameAllocator::BaseGetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
{
	if (!pthis)
		return MFX_ERR_INVALID_HANDLE;

	BaseFrameAllocator *self = reinterpret_cast<BaseFrameAllocator *>(pthis);

	return self->GetFrameHDL(mid, handle);
}

mfxStatus MFX_CDECL BaseFrameAllocator::BaseFree(mfxHDL pthis, mfxFrameAllocResponse *response)
{
	if (!pthis)
		return MFX_ERR_INVALID_HANDLE;

	BaseFrameAllocator *self = reinterpret_cast<BaseFrameAllocator *>(pthis);

	return self->FreeFrames(response);
}
