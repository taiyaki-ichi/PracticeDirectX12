#pragma once
#include"DescriptorHeap/DescriptorHeap.hpp"
#include"Resource/DoubleBufferResource.hpp"
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class DoubleBuffer
	{
		IDXGIFactory5* factory = nullptr;
		IDXGISwapChain4* swapChain = nullptr;
		DescriptorHeap<DescriptorHeapTypeTag::RTV> descriptorHeap{};

		std::array<DoubleBufferResource, 2> doubleBufferResources{};

	public:
		DoubleBuffer() = default;
		~DoubleBuffer();

		DoubleBuffer(const DoubleBuffer&) = delete;
		DoubleBuffer& operator=(const DoubleBuffer&) = delete;

		DoubleBuffer(DoubleBuffer&&) noexcept;
		DoubleBuffer& operator=(DoubleBuffer&&) noexcept;

		void Initialize(Device*, IDXGIFactory5*, IDXGISwapChain4*);

		D3D12_CPU_DESCRIPTOR_HANDLE GetBackbufferCpuHandle();

		void BarriorToBackbuffer(CommandList*, ResourceState);

		void ClearBackBuffer(CommandList*);

		void Flip();
	};

	//
	//
	//

	inline DoubleBuffer::~DoubleBuffer()
	{
		if (factory)
			factory->Release();
		if (swapChain)
			swapChain->Release();
	}

	inline DoubleBuffer::DoubleBuffer(DoubleBuffer&& rhs) noexcept
	{
		factory = rhs.factory;
		swapChain = rhs.swapChain;
		descriptorHeap = std::move(rhs.descriptorHeap);
		doubleBufferResources = std::move(rhs.doubleBufferResources);

		rhs.factory = nullptr;
		rhs.swapChain = nullptr;
	}

	inline DoubleBuffer& DoubleBuffer::operator=(DoubleBuffer&& rhs) noexcept
	{
		factory = rhs.factory;
		swapChain = rhs.swapChain;
		descriptorHeap = std::move(rhs.descriptorHeap);
		doubleBufferResources = std::move(rhs.doubleBufferResources);

		rhs.factory = nullptr;
		rhs.swapChain = nullptr;

		return *this;
	}

	inline void DoubleBuffer::Initialize(Device* device, IDXGIFactory5* f, IDXGISwapChain4* sc)
	{
		if (f == nullptr || sc == nullptr)
			throw "";

		factory = f;
		swapChain = sc;

		descriptorHeap.Initialize(device, 2);

		for (size_t i = 0; i < doubleBufferResources.size(); i++) {

			ID3D12Resource* resourcePtr;

			//失敗したとき
			if (FAILED(swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&resourcePtr))))
				throw "GetBuffer is failed\n";

			resourcePtr->SetName(L"DoubleBufferResource");
			doubleBufferResources[i].Initialize(resourcePtr);
			auto result = descriptorHeap.PushBackView(device, &doubleBufferResources[i]);
		}

	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE DoubleBuffer::GetBackbufferCpuHandle()
	{
		auto bbIdx = swapChain->GetCurrentBackBufferIndex();
		return descriptorHeap.GetCPUHandle(bbIdx);
	}

	inline void DoubleBuffer::BarriorToBackbuffer(CommandList* cl, ResourceState state)
	{
		auto bbIdx = swapChain->GetCurrentBackBufferIndex();
		doubleBufferResources[bbIdx].Barrior(cl, state);
	}

	inline void DoubleBuffer::ClearBackBuffer(CommandList* cl)
	{
		auto bbIdx = swapChain->GetCurrentBackBufferIndex();
		auto rtvH = descriptorHeap.GetCPUHandle(bbIdx);
		//バックバッファのクリア
		float clearColor[] = { 0.5f,0.5f,0.5f,1.0f };
		cl->Get()->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	}

	inline void DoubleBuffer::Flip()
	{
		swapChain->Present(1, 0);
	}



}