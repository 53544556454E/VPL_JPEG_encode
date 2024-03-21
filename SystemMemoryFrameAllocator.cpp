#include "SystemMemoryFrameAllocator.h"

SystemMemoryFrameAllocator::SystemMemoryFrameAllocator():
	BaseFrameAllocator()
{
}

SystemMemoryFrameAllocator::~SystemMemoryFrameAllocator()
{
}

mfxStatus SystemMemoryFrameAllocator::Initialize(FrameAllocatorParams *params)
{
	return MFX_ERR_NONE;
}

mfxStatus SystemMemoryFrameAllocator::Finalize()
{
	return MFX_ERR_NONE;
}

mfxStatus SystemMemoryFrameAllocator::AllocFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
	if (!request || !response)
		return MFX_ERR_NULL_PTR;

	if (WI_IsFlagClear(request->Type, MFX_MEMTYPE_SYSTEM_MEMORY))
		return MFX_ERR_UNSUPPORTED;

	if (kMaxWidth < request->Info.Width || kMaxHeight < request->Info.Height)
		return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;

	mfxU32 aligned_width = mfxUtl::Align(request->Info.Width, kWidthAlign);
	mfxU32 aligned_height = mfxUtl::Align(request->Info.Height, kHeightAlign);
	mfxU64 frame_size = mfxUtl::GetFrameSize(request->Info.FourCC, aligned_width, aligned_height);

	if (!frame_size)
		return MFX_ERR_UNSUPPORTED;

	mfxU32 num_allocated = 0;
	mfxMemId *mem_ids = new(std::nothrow) mfxMemId[request->NumFrameSuggested];

	if (!mem_ids)
		return MFX_ERR_MEMORY_ALLOC;

	for (num_allocated=0; num_allocated<request->NumFrameSuggested; ++num_allocated)
	{
		SystemMemoryFrameEntry *frame_entry = reinterpret_cast<SystemMemoryFrameEntry *>(new (std::nothrow) mfxU8[mfxUtl::Align(sizeof (SystemMemoryFrameEntry), kFrameAlign) + frame_size]);

		if (frame_entry)
			frame_entry->frame_info_ = request->Info;
		else
			break;

		mem_ids[num_allocated] = reinterpret_cast<mfxMemId>(frame_entry);
	}

	if (num_allocated != request->NumFrameSuggested)
	{
		for (mfxU32 i=0; i<num_allocated; ++i)
			delete mem_ids[i];

		delete[] mem_ids;

		return MFX_ERR_MEMORY_ALLOC;
	}

	response->NumFrameActual = static_cast<mfxU16>(num_allocated);
	response->mids = mem_ids;

	return MFX_ERR_NONE;
}

mfxStatus SystemMemoryFrameAllocator::LockFrame(mfxMemId mid, mfxFrameData *ptr)
{
	if (!mid)
		return MFX_ERR_INVALID_HANDLE;

	if (!ptr)
		return MFX_ERR_NULL_PTR;

	SystemMemoryFrameEntry *frame_entry = reinterpret_cast<SystemMemoryFrameEntry *>(mid);

	mfxU32 four_cc = frame_entry->frame_info_.FourCC;
	mfxU8 *frame_base = reinterpret_cast<mfxU8 *>(mid) + mfxUtl::Align(sizeof (SystemMemoryFrameEntry), kFrameAlign);
	mfxU32 width = mfxUtl::Align(frame_entry->frame_info_.Width, kWidthAlign);
	mfxU32 height = mfxUtl::Align(frame_entry->frame_info_.Height, kHeightAlign);

	return UpdateLockedFrame(ptr, four_cc, frame_base, width, height);
}

mfxStatus SystemMemoryFrameAllocator::UnlockFrame(mfxMemId mid, mfxFrameData *ptr)
{
	if (!mid)
		return MFX_ERR_INVALID_HANDLE;

	// IntelのサンプルではmfxFrameData *ptrがnullptrの場合を許容するので、それに準じた
	UpdateUnlockedFrame(ptr);

	return MFX_ERR_NONE;
}

mfxStatus SystemMemoryFrameAllocator::GetFrameHDL(mfxMemId mid, mfxHDL *handle)
{
	return MFX_ERR_UNSUPPORTED;
}

mfxStatus SystemMemoryFrameAllocator::FreeFrames(mfxFrameAllocResponse *response)
{
	if (!response)
		return MFX_ERR_NULL_PTR;

	if (!response->mids)
		return MFX_ERR_NONE;

	for (mfxU32 i=0; i<response->NumFrameActual; ++i)
	{
		delete response->mids[i];
		response->mids[i] = nullptr;
	}

	response->NumFrameActual = 0;
	delete[] response->mids;
	response->mids = nullptr;

	return MFX_ERR_NONE;
}
