#pragma once
#include"Resource/frame_buffer_resource.hpp"
#include"Command.hpp"
#include"PipelineState.hpp"
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	class swap_chain
	{
		release_unique_ptr<IDXGISwapChain3> swap_chain_ptr;

		std::array<frame_buffer_resource<FrameBufferFormat>, FrameBufferNum> frameBuffer;

	public:
		swap_chain() = default;
		~swap_chain() = default;

		swap_chain<FrameBufferFormat, FrameBufferNum>(swap_chain<FrameBufferFormat, FrameBufferNum>&&) = default;
		swap_chain<FrameBufferFormat, FrameBufferNum>& operator=(swap_chain<FrameBufferFormat, FrameBufferNum>&&) = default;

		template<typename Command>
		void initialize(Command&, HWND);

		//レンダリングされた画像を表示する
		//また、GetCurrentBackBufferIndexの戻り値が更新される
		void present(std::uint32_t syncInterval = 1);

		//現在控えているBackBufferのインデックスの取得
		std::uint32_t get_vcurrent_back_buffer_index();

		frame_buffer_resource<FrameBufferFormat>& get_frame_buffer(std::uint32_t index);
	};

	//
	//
	//


	template<typename FrameBufferFormat, std::size_t FrameBufferNum>
	template<typename Command>
	inline void swap_chain<FrameBufferFormat, FrameBufferNum>::initialize(Command& command, HWND hwnd)
	{
		IDXGIFactory3* factory = nullptr;
		IDXGISwapChain4* swapChain = nullptr;

#ifdef _DEBUG
		if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory))))
			THROW_EXCEPTION("");
#else
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
			throw "";
#endif

		RECT windowRect{};
		GetWindowRect(hwnd, &windowRect);

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
		swapchainDesc.Width = windowRect.right - windowRect.left;
		swapchainDesc.Height = windowRect.bottom - windowRect.top;
		swapchainDesc.Format = get_dxgi_format(FrameBufferFormat::component_type, FrameBufferFormat::component_size, FrameBufferFormat::component_num).value();
		swapchainDesc.Stereo = false;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
		swapchainDesc.BufferCount = FrameBufferNum;
		swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		if (FAILED(factory->CreateSwapChainForHwnd(command.get_queue(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain)))
			THROW_EXCEPTION("");

		swap_chain_ptr.reset(swapChain);
		factory->Release();

		for (std::uint32_t i = 0; i < FrameBufferNum; i++)
		{
			ID3D12Resource* resourcePtr = nullptr;
			if (FAILED(swap_chain_ptr->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&resourcePtr))))
				THROW_EXCEPTION("");
			frameBuffer[i].initialize(resourcePtr);
		}
	}


	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline void swap_chain<FrameBufferFormat,FrameBufferNum>::present(std::uint32_t syncInterval)
	{
		swap_chain_ptr->Present(syncInterval, 0);
	}

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline std::uint32_t swap_chain<FrameBufferFormat,FrameBufferNum>::get_vcurrent_back_buffer_index()
	{
		return swap_chain_ptr->GetCurrentBackBufferIndex();
	}


	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline frame_buffer_resource<FrameBufferFormat>& DX12::swap_chain<FrameBufferFormat,FrameBufferNum>::get_frame_buffer(std::uint32_t index)
	{
		return frameBuffer[index];
	}
}