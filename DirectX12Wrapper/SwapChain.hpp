#pragma once
#include"Resource/FrameBufferResource.hpp"
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
		IDXGISwapChain3* const swapChain;
		std::array<FrameBufferResource, FrameBufferNum> frameBuffer{};


	public:
		SwapChain(IDXGISwapChain3*);

		//レンダリングされた画像を表示する
		//また、GetCurrentBackBufferIndexの戻り値が更新される
		void Present();

		//現在控えているBackBufferのインデックスの取得
		std::uint32_t GetCurrentBackBufferIndex();

		FrameBufferResource& GetFrameBuffer(std::size_t index);
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
				throw "GetBuffer is failed\n";
			frameBuffer[i].Initialize(resourcePtr);
		}
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
	inline FrameBufferResource& DX12::SwapChain<FrameBufferNum>::GetFrameBuffer(std::size_t index)
	{
		return frameBuffer[index];
	}
}