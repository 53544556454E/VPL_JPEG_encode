#pragma once

#include <new>

#include <wil/common.h>

#include "BaseFrameAllocator.h"
#include "mfxutl.h"

class SystemMemoryFrameAllocator:
	public BaseFrameAllocator
{
public:
	SystemMemoryFrameAllocator();
	virtual ~SystemMemoryFrameAllocator();

	virtual mfxStatus Initialize(FrameAllocatorParams *params) override;
	virtual mfxStatus Finalize() override;

	virtual mfxStatus AllocFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response) override;
	virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData *ptr) override;
	virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData *ptr) override;
	virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle) override;
	virtual mfxStatus FreeFrames(mfxFrameAllocResponse *response) override;

private:
	static const mfxU32 kMaxWidth = 16384;
	static const mfxU32 kMaxHeight = 16384;

	// SystemMemoryFrameEntryの直後のkFrameAlignバイト整列した位置にフレームの実体を配置する
	static const mfxU32 kFrameAlign = 64;

	// メモリ確保およびロックする際、幅と高さを指定サイズの自然数倍に切り上げる
	static const mfxU32 kWidthAlign = 16;
	static const mfxU32 kHeightAlign = 16;

	// mfxFrameAllocResponse::mid[] で保持するポインタで、LockFrame, UnlockFrame, 及び GetFrameHDL の引数で与えられる mfxMemId が指す構造体
	struct SystemMemoryFrameEntry
	{
		mfxFrameInfo frame_info_; // フレーム割当時に記録し、フレームをロックする際に参照する
	};
};
