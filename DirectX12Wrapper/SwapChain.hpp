#pragma once
#include"Resource/frame_buffer_resource.hpp"
//
#include"PipelineState.hpp"
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	class SwapChain
	{
		release_unique_ptr<IDXGISwapChain3> swap_chain_ptr;

		std::array<frame_buffer_resource<FrameBufferFormat>, FrameBufferNum> frameBuffer;

	public:
		SwapChain(IDXGISwapChain3*);
		~SwapChain() = default;

		SwapChain<FrameBufferFormat, FrameBufferNum>(SwapChain<FrameBufferFormat, FrameBufferNum>&&) = default;
		SwapChain<FrameBufferFormat, FrameBufferNum>& operator=(SwapChain<FrameBufferFormat, FrameBufferNum>&&) = default;

		//レンダリングされた画像を表示する
		//また、GetCurrentBackBufferIndexの戻り値が更新される
		void Present(std::uint32_t syncInterval = 1);

		//現在控えているBackBufferのインデックスの取得
		std::uint32_t GetCurrentBackBufferIndex();

		frame_buffer_resource<FrameBufferFormat>& GetFrameBuffer(std::uint32_t index);
	};

	//
	//
	//

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferFormat,FrameBufferNum>::SwapChain(IDXGISwapChain3* sc)
		:swap_chain_ptr{sc}
		, frameBuffer{}
	{
		for (std::uint32_t i = 0; i < FrameBufferNum; i++)
		{
			ID3D12Resource* resourcePtr = nullptr;
			if (FAILED(swap_chain_ptr->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&resourcePtr))))
				throw "";
			frameBuffer[i].initialize(resourcePtr);
		}
	}

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline void SwapChain<FrameBufferFormat,FrameBufferNum>::Present(std::uint32_t syncInterval)
	{
		swap_chain_ptr->Present(syncInterval, 0);
	}

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline std::uint32_t SwapChain<FrameBufferFormat,FrameBufferNum>::GetCurrentBackBufferIndex()
	{
		return swap_chain_ptr->GetCurrentBackBufferIndex();
	}


	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline frame_buffer_resource<FrameBufferFormat>& DX12::SwapChain<FrameBufferFormat,FrameBufferNum>::GetFrameBuffer(std::uint32_t index)
	{
		return frameBuffer[index];
	}
}