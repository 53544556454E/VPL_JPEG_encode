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

	// SystemMemoryFrameEntry�̒����kFrameAlign�o�C�g���񂵂��ʒu�Ƀt���[���̎��̂�z�u����
	static const mfxU32 kFrameAlign = 64;

	// �������m�ۂ���у��b�N����ہA���ƍ������w��T�C�Y�̎��R���{�ɐ؂�グ��
	static const mfxU32 kWidthAlign = 16;
	static const mfxU32 kHeightAlign = 16;

	// mfxFrameAllocResponse::mid[] �ŕێ�����|�C���^�ŁALockFrame, UnlockFrame, �y�� GetFrameHDL �̈����ŗ^������ mfxMemId ���w���\����
	struct SystemMemoryFrameEntry
	{
		mfxFrameInfo frame_info_; // �t���[���������ɋL�^���A�t���[�������b�N����ۂɎQ�Ƃ���
	};
};
