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
		IDXGISwapChain3* swapChain;

		std::array<frame_buffer_resource<FrameBufferFormat>, FrameBufferNum> frameBuffer{};

	public:
		SwapChain(IDXGISwapChain3*);
		~SwapChain();

		SwapChain<FrameBufferFormat,FrameBufferNum>(SwapChain<FrameBufferFormat,FrameBufferNum> const&) = delete;
		SwapChain<FrameBufferFormat,FrameBufferNum>& operator=(SwapChain<FrameBufferFormat,FrameBufferNum> const&) = delete;

		SwapChain<FrameBufferFormat,FrameBufferNum>(SwapChain<FrameBufferFormat,FrameBufferNum>&&) noexcept;
		SwapChain<FrameBufferFormat,FrameBufferNum>& operator=(SwapChain<FrameBufferFormat,FrameBufferNum>&&) noexcept;

		//レンダリングされた画像を表示する
		//また、GetCurrentBackBufferIndexの戻り値が更新される
		void Present();

		//現在控えているBackBufferのインデックスの取得
		std::uint32_t GetCurrentBackBufferIndex();

		frame_buffer_resource<FrameBufferFormat>& GetFrameBuffer(std::uint32_t index);
	};

	//
	//
	//

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferFormat,FrameBufferNum>::SwapChain(IDXGISwapChain3* sc)
		:swapChain{sc}
		, frameBuffer{}
	{
		for (std::uint32_t i = 0; i < FrameBufferNum; i++)
		{
			ID3D12Resource* resourcePtr = nullptr;
			if (FAILED(swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&resourcePtr))))
				throw "";
			frameBuffer[i].initialize(resourcePtr);
		}
	}

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferFormat, FrameBufferNum>::~SwapChain()
	{
		if (swapChain)
			swapChain->Release();
	}

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferFormat, FrameBufferNum>::SwapChain(SwapChain<FrameBufferFormat, FrameBufferNum>&& rhs) noexcept
	{
		swapChain = rhs.swapChain;
		rhs.swapChain = nullptr;
		frameBuffer = std::move(rhs.frameBuffer);
	}

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferFormat,FrameBufferNum>& SwapChain<FrameBufferFormat,FrameBufferNum>::operator=(SwapChain<FrameBufferFormat,FrameBufferNum>&& rhs) noexcept
	{
		swapChain = rhs.swapChain;
		rhs.swapChain = nullptr;
		frameBuffer = std::move(rhs.frameBuffer);
		return *this;
	}

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline void SwapChain<FrameBufferFormat,FrameBufferNum>::Present()
	{
		swapChain->Present(1, 0);
	}

	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline std::uint32_t SwapChain<FrameBufferFormat,FrameBufferNum>::GetCurrentBackBufferIndex()
	{
		return swapChain->GetCurrentBackBufferIndex();
	}


	template<typename FrameBufferFormat,std::size_t FrameBufferNum>
	inline frame_buffer_resource<FrameBufferFormat>& DX12::SwapChain<FrameBufferFormat,FrameBufferNum>::GetFrameBuffer(std::uint32_t index)
	{
		return frameBuffer[index];
	}
}