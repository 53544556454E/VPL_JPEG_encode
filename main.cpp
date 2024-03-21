#include <Windows.h>

#include <d3d11.h>
#include <dxgi.h>

#include <fcntl.h>
#include <io.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <iostream>

#include <wil/com.h>
#include <wil/resource.h>

#include <vpl/mfx.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "mfxutl.h"
#include "Stopwatch.h"
#include "SystemMemoryFrameAllocator.h"

void PrintAdapters();
bool FillFrameSurface(mfxFrameSurface1 *frame_surface, BaseFrameAllocator *allocator, int width, int height, const void *image);

int main(int argc, char *argv[])
{
	wil::unique_couninitialize_call com = wil::CoInitializeEx(COINIT_MULTITHREADED| COINIT_DISABLE_OLE1DDE);

	if (_setmode(_fileno(stdout), _O_BINARY) == -1)
	{
		std::cerr << "Cannot set stdout to binary mode." << std::endl;
		return -1;
	}

	if (argc < 3)
	{
		PrintAdapters();
		std::string file_name = std::filesystem::path(argv[0]).filename().string();
		std::cout << std::format("{:s} adapter_index input_file [quality]", file_name) << std::endl;

		return 0;
	}

	int adapter_index = std::atoi(argv[1]);
	char *input_file_path = argv[2];
	int quality = (argc == 3)? 100: std::clamp(std::atoi(argv[3]), 1, 100);
	int width;
	int height;
	int comp;
	stbi_uc *image = stbi_load(input_file_path, &width, &height, &comp, STBI_rgb);

	if (!image)
	{
		std::cerr << "Can not load input_file." << std::endl;

		return -1;
	}

	if (comp != 3)
	{
		std::cerr << "Only three component files are supported." << std::endl;

		return -1;
	}

	Stopwatch sw;
	mfxStatus status;

	sw.Start();
	mfxLoader loader = MFXLoad();
	sw.Stop("MFXLoad:", Stopwatch::Duration::MicroSeconds);

	sw.Start();
	mfxConfig config = MFXCreateConfig(loader);
	sw.Stop("MFXCreateConfig:", Stopwatch::Duration::MicroSeconds);

	mfxSession session;
	mfxVariant var {};

	var.Type = MFX_VARIANT_TYPE_U32;
	var.Data.U32 = std::atoi(argv[1]);
	
	sw.Start();
	MFXSetConfigFilterProperty(config, reinterpret_cast<const mfxU8 *>("DXGIAdapterIndex"), var);
	sw.Stop("MFXSetConfigFilterProperty:", Stopwatch::Duration::MicroSeconds);

	var.Type = MFX_VARIANT_TYPE_U32;
	var.Data.U32 = MFX_CODEC_JPEG;

	sw.Start();
	MFXSetConfigFilterProperty(config, reinterpret_cast<const mfxU8 *>("mfxImplDescription.mfxEncoderDescription.encoder.CodecID"), var);
	sw.Stop("MFXSetConfigFilterProperty:", Stopwatch::Duration::MicroSeconds);

	sw.Start();
	status = MFXCreateSession(loader, 0, &session);
	sw.Stop("MFXCreateSession:", Stopwatch::Duration::MicroSeconds);

	if (status != MFX_ERR_NONE)
	{
		std::cerr << "Adapter does not support JPEG encoding." << std::endl;

		return -1;
	}

	SystemMemoryFrameAllocator sys_alloc;
	
	sw.Start();
	MFXVideoCORE_SetFrameAllocator(session, &sys_alloc);
	sw.Stop("MFXVideoCORE_SetFrameAllocator:", Stopwatch::Duration::MicroSeconds);

	mfxVideoParam encode_params {};
	encode_params.mfx.CodecId = MFX_CODEC_JPEG;
	encode_params.mfx.FrameInfo.FourCC = MFX_FOURCC_YUY2;
	encode_params.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV422;
	encode_params.mfx.FrameInfo.Width = mfxUtl::Align(width, 16);
	encode_params.mfx.FrameInfo.Height = mfxUtl::Align(height, 16);
	encode_params.mfx.FrameInfo.CropW = width;
	encode_params.mfx.FrameInfo.CropH = height;
	encode_params.mfx.FrameInfo.AspectRatioW = 1;
	encode_params.mfx.FrameInfo.AspectRatioH = 1;
	encode_params.mfx.FrameInfo.FrameRateExtN = 1;
	encode_params.mfx.FrameInfo.FrameRateExtD = 1;
	encode_params.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	encode_params.mfx.CodecProfile = MFX_PROFILE_JPEG_BASELINE;
	encode_params.mfx.Interleaved = MFX_SCANTYPE_INTERLEAVED;
	encode_params.mfx.Quality = quality;
	encode_params.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

	sw.Start();
	status = MFXVideoENCODE_Query(session, &encode_params, &encode_params);
	sw.Stop("MFXVideoENCODE_Query:", Stopwatch::Duration::MicroSeconds);

	if (status != MFX_ERR_NONE)
		std::cerr << "MFXVideoENCODE_Query: " << status << std::endl;

	mfxVideoParam vpp_params {};
	vpp_params.vpp.In = encode_params.mfx.FrameInfo;
	vpp_params.vpp.In.FourCC = MFX_FOURCC_RGB4;
	vpp_params.vpp.In.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
	vpp_params.vpp.Out = encode_params.mfx.FrameInfo;
	vpp_params.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY| MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

	mfxExtVPPVideoSignalInfo video_signal_info = {};
	video_signal_info.Header.BufferId = MFX_EXTBUFF_VPP_VIDEO_SIGNAL_INFO;
	video_signal_info.Header.BufferSz = sizeof (video_signal_info);
	video_signal_info.TransferMatrix = MFX_TRANSFERMATRIX_BT601;
	video_signal_info.NominalRange = MFX_NOMINALRANGE_16_235;

	mfxExtBuffer *ext[1] = {reinterpret_cast<mfxExtBuffer *>(&video_signal_info)};

	vpp_params.NumExtParam = 1;
	vpp_params.ExtParam = ext;

	sw.Start();
	status = MFXVideoVPP_Query(session, &vpp_params, &vpp_params);
	sw.Stop("MFXVideoVPP_Query:", Stopwatch::Duration::MicroSeconds);

	if (status != MFX_ERR_NONE)
		std::cerr << "MFXVideoVPP_Query: " << status << std::endl;

	mfxFrameAllocRequest vpp_alloc_request[2] {};
	mfxFrameAllocResponse vpp_alloc_response[2] {};

	sw.Start();
	status = MFXVideoVPP_QueryIOSurf(session, &vpp_params, vpp_alloc_request);
	sw.Stop("MFXVideoVPP_QueryIOSurf:", Stopwatch::Duration::MicroSeconds);

	if (status != MFX_ERR_NONE)
		std::cerr << "MFXVideoVPP_QueryIOSurf: " << status << std::endl;

	sw.Start();
	status = MFXVideoVPP_Init(session, &vpp_params);
	sw.Stop("MFXVideoVPP_Init:", Stopwatch::Duration::MicroSeconds);

	if (status != MFX_ERR_NONE)
		std::cerr << "MFXVideoVPP_Init: " << status << std::endl;

	sw.Start();
	status = MFXVideoENCODE_Init(session, &encode_params);
	sw.Stop("MFXVideoENCODE_Init:", Stopwatch::Duration::MicroSeconds);

	if (status != MFX_ERR_NONE)
		std::cerr << "MFXVideoENCODE_Init: " << status << std::endl;

	sw.Start();
	status = sys_alloc.AllocFrames(&vpp_alloc_request[0], &vpp_alloc_response[0]);
	status = sys_alloc.AllocFrames(&vpp_alloc_request[1], &vpp_alloc_response[1]);
	sw.Stop("AllocFrames:", Stopwatch::Duration::MicroSeconds);

	mfxFrameSurface1 vpp_input_surface {};
	mfxFrameSurface1 vpp_output_surface {};

	vpp_input_surface.Data.MemType = vpp_alloc_request[0].Type;
	vpp_input_surface.Data.MemId = vpp_alloc_response[0].mids[0];
	vpp_input_surface.Info = vpp_alloc_request[0].Info;

	vpp_output_surface.Data.MemType = vpp_alloc_request[1].Type;
	vpp_output_surface.Data.MemId = vpp_alloc_response[1].mids[0];
	vpp_output_surface.Info = vpp_alloc_request[1].Info;

	sw.Start();
	FillFrameSurface(&vpp_input_surface, &sys_alloc, width, height, image);
	sw.Stop("FillFrameSurface:", Stopwatch::Duration::MicroSeconds);

	mfxSyncPoint sync {};

	sw.Start();
	status = MFXVideoVPP_RunFrameVPPAsync(session, &vpp_input_surface, &vpp_output_surface, nullptr, &sync);
	sw.Stop("MFXVideoVPP_RunFrameVPPAsync:", Stopwatch::Duration::MicroSeconds);

	if (status != MFX_ERR_NONE)
		std::cerr << "MFXVideoVPP_RunFrameVPPAsync: " << status << std::endl;

	mfxBitstream bit_stream {};
	std::vector<mfxU8> bit_stream_data(width * height * 3 + 8192);

	bit_stream.Data = bit_stream_data.data();
	bit_stream.MaxLength = static_cast<mfxU32>(bit_stream_data.size());

	sw.Start();
	status = MFXVideoENCODE_EncodeFrameAsync(session, nullptr, &vpp_output_surface, &bit_stream, &sync);
	sw.Stop("MFXVideoENCODE_EncodeFrameAsync:", Stopwatch::Duration::MicroSeconds);
	
	if (status != MFX_ERR_NONE)
		std::cerr << "MFXVideoENCODE_EncodeFrameAsync: " << status << std::endl;

	sw.Start();
	while (true)
	{
		if (!sync)
			break;

		status = MFXVideoCORE_SyncOperation(session, sync, 1000);
		if (status != MFX_WRN_IN_EXECUTION)
			break;
	}
	sw.Stop("MFXVideoCORE_SyncOperation:", Stopwatch::Duration::MicroSeconds);

	if (status == MFX_ERR_NONE)
	{
		std::cout.write(reinterpret_cast<char *>(bit_stream.Data), bit_stream.DataLength);
		std::cout.flush();
	}
	else
		std::cerr << "MFXVideoCORE_SyncOperation: " << status << std::endl;

	sw.Start();
	MFXVideoENCODE_Close(session);
	MFXVideoVPP_Close(session);
	MFXClose(session);
	MFXUnload(loader);
	sys_alloc.FreeFrames(&vpp_alloc_response[1]);
	sys_alloc.FreeFrames(&vpp_alloc_response[0]);
	sw.Stop("finalize function calls:", Stopwatch::Duration::MicroSeconds);

	return 0;
}

void PrintAdapters()
{
	wil::com_ptr<IDXGIFactory> factory;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));

	if (hr != S_OK)
	{
		std::cerr << std::format("CreateDXGIFactory: {:x}", hr) << std::endl;
		return;
	}

	std::cout << "adapter_index: adapter_description" << std::endl;

	for (UINT idx=0; ; ++idx)
	{
		wil::com_ptr<IDXGIAdapter> adapter;
		DXGI_ADAPTER_DESC desc {};

		if (factory->EnumAdapters(idx, &adapter) != S_OK)
			break;

		adapter->GetDesc(&desc);
		std::wcout << std::format(L"{}: {}", idx, desc.Description) << std::endl;
	}

	std::cout << std::endl;
}

bool FillFrameSurface(mfxFrameSurface1 *frame_surface, BaseFrameAllocator *allocator, int width, int height, const void *image)
{
	mfxStatus status;

	status = allocator->Lock(allocator, frame_surface->Data.MemId, &frame_surface->Data);
	if (status != MFX_ERR_NONE)
		return false;

	mfxU32 pitch = (mfxU32(frame_surface->Data.PitchHigh) << 16) + frame_surface->Data.PitchLow;

	// RGB to MFX_FOURCC_RGB4(BGRA)
	for (int y=0; y<height; ++y)
	{
		const uint8_t *src = static_cast<const uint8_t *>(image) + ((3 * width) * y);
		uint8_t *dst_r = frame_surface->Data.R + (pitch * y);
		uint8_t *dst_g = frame_surface->Data.G + (pitch * y);
		uint8_t *dst_b = frame_surface->Data.B + (pitch * y);

		for (int x=0; x<width; ++x)
		{
			*dst_r = src[0];
			*dst_g = src[1];
			*dst_b = src[2];

			dst_r += 4;
			dst_g += 4;
			dst_b += 4;
			src += 3;
		}

		// YUY2に変換する際、右端が正しくなるようコピーする
		if (width & 1)
		{
			src -= 3;
			*dst_r = src[0];
			*dst_g = src[1];
			*dst_b = src[2];
		}
	}

	allocator->Unlock(allocator, frame_surface->Data.MemId, &frame_surface->Data);
	if (status != MFX_ERR_NONE)
		return false;

	return true;
}
