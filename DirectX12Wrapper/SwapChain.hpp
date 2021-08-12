#pragma once
#include"Resource/FrameBuffer.hpp"
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{

	template<std::size_t FrameBufferNum = 2>
	class SwapChain
	{
		IDXGISwapChain3* swapChain;
		std::array<FrameBuffer, FrameBufferNum> frameBuffer{};

	public:
		SwapChain(IDXGISwapChain3*);
		~SwapChain();

		SwapChain<FrameBufferNum>(SwapChain<FrameBufferNum> const&) = delete;
		SwapChain<FrameBufferNum>& operator=(SwapChain<FrameBufferNum> const&) = delete;

		SwapChain<FrameBufferNum>(SwapChain<FrameBufferNum>&&) noexcept;
		SwapChain<FrameBufferNum>& operator=(SwapChain<FrameBufferNum>&&) noexcept;

		//レンダリングされた画像を表示する
		//また、GetCurrentBackBufferIndexの戻り値が更新される
		void Present();

		//現在控えているBackBufferのインデックスの取得
		std::uint32_t GetCurrentBackBufferIndex();

		FrameBuffer& GetFrameBuffer(std::size_t index);
	};

	//
	//
	//

	template<std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferNum>::SwapChain(IDXGISwapChain3* sc)
		:swapChain{sc}
		, frameBuffer{}
	{
		for (std::uint32_t i = 0; i < FrameBufferNum; i++)
		{
			ID3D12Resource* resourcePtr = nullptr;
			if (FAILED(swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&resourcePtr))))
				throw "";
			frameBuffer[i].Initialize(resourcePtr);
		}
	}

	template<std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferNum>::~SwapChain()
	{
		if (swapChain)
			swapChain->Release();
	}

	template<std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferNum>::SwapChain(SwapChain<FrameBufferNum>&& rhs) noexcept
	{
		swapChain = rhs.swapChain;
		rhs.swapChain = nullptr;
		frameBuffer = std::move(rhs.frameBuffer);
	}

	template<std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferNum>& SwapChain<FrameBufferNum>::operator=(SwapChain<FrameBufferNum>&& rhs) noexcept
	{
		swapChain = rhs.swapChain;
		rhs.swapChain = nullptr;
		frameBuffer = std::move(rhs.frameBuffer);
		return *this;
	}

	template<std::size_t FrameBufferNum>
	inline void SwapChain<FrameBufferNum>::Present()
	{
		swapChain->Present(1, 0);
	}

	template<std::size_t FrameBufferNum>
	inline std::uint32_t SwapChain<FrameBufferNum>::GetCurrentBackBufferIndex()
	{
		return swapChain->GetCurrentBackBufferIndex();
	}

	template<std::size_t FrameBufferNum>
	inline FrameBuffer& DX12::SwapChain<FrameBufferNum>::GetFrameBuffer(std::size_t index)
	{
		return frameBuffer[index];
	}
}