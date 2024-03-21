#pragma once

#include <vpl/mfxdefs.h>
#include <vpl/mfxvideo.h>

#include "mfxutl.h"

class BaseFrameAllocator:
	public mfxFrameAllocator
{
public:
	BaseFrameAllocator();
	virtual ~BaseFrameAllocator();

	struct FrameAllocatorParams
	{
		virtual ~FrameAllocatorParams()
		{
		}
	};

	virtual mfxStatus Initialize(FrameAllocatorParams *params) = 0;
	virtual mfxStatus Finalize() = 0;

	virtual mfxStatus AllocFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response) = 0;
	virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData *ptr) = 0;
	virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData *ptr) = 0;
	virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle) = 0;
	virtual mfxStatus FreeFrames(mfxFrameAllocResponse *response) = 0;

protected:
	mfxStatus UpdateLockedFrame(mfxFrameData *ptr, mfxU32 four_cc, mfxU8 *frame_base, mfxU32 width, mfxU32 height);
	void UpdateUnlockedFrame(mfxFrameData *ptr);

private:
	static mfxStatus MFX_CDECL BaseAlloc(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
	static mfxStatus MFX_CDECL BaseLock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
	static mfxStatus MFX_CDECL BaseUnlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
	static mfxStatus MFX_CDECL BaseGetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
	static mfxStatus MFX_CDECL BaseFree(mfxHDL pthis, mfxFrameAllocResponse *response);
};
