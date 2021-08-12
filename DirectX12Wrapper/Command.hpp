#pragma once
#include"Device.hpp"
#include"SwapChain.hpp"
#include"PipelineState.hpp"
#include"Resource/VertexBuffer.hpp"
#include"Resource/IndexBuffer.hpp"
#include"DescriptorHeap/DescriptorHeap.hpp"
#include<array>
#include<algorithm>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	template<std::size_t FrameLatencyNum=2>
	class Command
	{
		ID3D12CommandQueue* queue = nullptr;
		ID3D12GraphicsCommandList* list = nullptr;

		std::array<ID3D12CommandAllocator*, FrameLatencyNum> allocator{};
		std::array<ID3D12Fence*, FrameLatencyNum> fence{};
		std::array<std::uint64_t, FrameLatencyNum> fenceValue{};
		HANDLE fenceEventHandle = nullptr;

	public:
		Command() = default;
		~Command();

		Command(const  Command<FrameLatencyNum>&) = delete;
		Command& operator=(const Command<FrameLatencyNum>&) = delete;

		Command(Command<FrameLatencyNum>&&) noexcept;
		Command& operator=(Command<FrameLatencyNum>&&) noexcept;

		void Initialize(Device*);

		template<std::size_t FrameBufferNum=FrameLatencyNum>
		SwapChain<FrameBufferNum> CreateSwapChain(Device*, HWND);

		void CopyTexture(Device*,ResourceBase* srcResource, ResourceBase* dstResource);

		void Execute();
		void Dispatch(std::uint32_t threadGroupCountX, std::uint32_t threadGroupCountY, std::uint32_t threadGroupCountZ);

		//index番目のfenceValueを更新し、Queueにその値をSignalする
		//つまりWaitで待たなければならない位置を記録しておく、的な
		void Fence(std::uint64_t index);

		//index番目のコマンドの終了を待つ
		void Wait(std::uint64_t index);
		//現在発行されている全てのコマンドの処理を待つ
		//いらないかも
		void WaitAll(Device*);

		//indexはアロケータのインデックス
		void Reset(std::size_t index);

		void Close();


		void SetPipelineState(PipelineState*);

		//OMSetRenderTargetsの最適化について、どのターゲットのViewもおなじディスクリプタヒープ連続して生成されていないとみなしている
		void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle);
		void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle);
		void SetRenderTarget(std::uint32_t renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget);
		void SetRenderTarget(std::uint32_t renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle);

		void SetViewport(const D3D12_VIEWPORT& viewport);
		void SetViewport(std::uint32_t num, D3D12_VIEWPORT* viewportPtr);

		void SetScissorRect(const D3D12_RECT& rect);
		void SetScissorRect(std::uint32_t num, D3D12_RECT* rectPtr);

		void SetVertexBuffer(VertexBuffer*);
		void SetIndexBuffer(IndexBuffer*);

		void SetGraphicsRootSignature(RootSignature*);
		void SetComputeRootSignature(RootSignature*);

		template<typename T>
		void SetDescriptorHeap(DescriptorHeap<T>*);
		void SetGraphicsRootDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE);
		void SetComputeRootDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE);

		void SetPrimitiveTopology(PrimitiveTopology);

		void DrawInstanced(std::uint32_t vertexNumPerInstance, std::uint32_t instanceNum = 1);
		void DrawIndexedInstanced(std::uint32_t indexNumPerInstance, std::uint32_t instanceNum = 1);

		void Barrior(ResourceBase*, ResourceState);

		void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE, const std::array<float, 4>&);
		void ClearDepthView(D3D12_CPU_DESCRIPTOR_HANDLE, float);
	};


	//
	//
	//



	template<std::size_t FrameLatencyNum>
	inline Command<FrameLatencyNum>::~Command()
	{
		for (auto a : allocator)
			if (a)
				a->Release();
		if (queue)
			queue->Release();
		if (list)
			list->Release();
		for (auto f : fence)
			if (f)
				f->Release();
	}

	template<std::size_t FrameLatencyNum>
	inline Command<FrameLatencyNum>::Command(Command<FrameLatencyNum>&& rhs) noexcept
		:queue{rhs.queue}
		, list{rhs.list}
		, allocator{rhs.allocator}
		, fence{rhs.fence}
		, fenceValue{rhs.fenceValue}
		, fenceEventHandle{rhs.fenceEventHandle}
	{
		rhs.queue = nullptr;
		rhs.list = nullptr;
		std::fill(rhs.allocator.begin(), rhs.allocator.end(), nullptr);
		std::fill(rhs.allocator.begin(), rhs.allocator.end(), nullptr);
		rhs.fenceEventHandle = nullptr;
	}

	template<std::size_t FrameLatencyNum>
	inline Command<FrameLatencyNum>& Command<FrameLatencyNum>::operator=(Command<FrameLatencyNum>&& rhs) noexcept
	{
		queue = rhs.queue;
		list = rhs.list;
		allocator = rhs.allocator;
		fence = rhs.fence;
		fenceValue = rhs.fenceValue;
		fenceEventHandle = rhs.fenceEventHandle;
		rhs.queue = nullptr;
		rhs.list = nullptr;
		std::fill(rhs.allocator.begin(), rhs.allocator.end(), nullptr);
		std::fill(rhs.allocator.begin(), rhs.allocator.end(), nullptr);
		rhs.fenceEventHandle = nullptr;
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Initialize(Device* device)
	{
		for (auto& a : allocator)
			if (FAILED(device->Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&a))))
				throw "";

		if (FAILED(device->Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator[0], nullptr, IID_PPV_ARGS(&list))))
			throw "";

		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
		cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//タイムアウトナシ
		cmdQueueDesc.NodeMask = 0;
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//プライオリティ特に指定なし
		cmdQueueDesc.Type = list->GetType();			//ここはコマンドリストと合わせる
		if (FAILED(device->Get()->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&queue))))
			throw "";

		//とりあえず全て１で初期化してみる
		std::fill(fenceValue.begin(), fenceValue.end(), 1);

		for (std::size_t i = 0; i < FrameLatencyNum; i++)
			if (FAILED(device->Get()->CreateFence(fenceValue[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]))))
				throw "";

		fenceEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);

		Close();//必要
	}


	template<std::size_t FrameLatencyNum>
	template<std::size_t FrameBufferNum>
	inline SwapChain<FrameBufferNum> Command<FrameLatencyNum>::CreateSwapChain(Device* device, HWND hwnd)
	{
		IDXGIFactory3* factory = nullptr;
		IDXGISwapChain4* swapChain = nullptr;

#ifdef _DEBUG
		if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory))))
			throw "";
#else
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
			throw "";
#endif

		RECT windowRect{};
		GetWindowRect(hwnd, &windowRect);

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
		swapchainDesc.Width = windowRect.right - windowRect.left;
		swapchainDesc.Height = windowRect.bottom - windowRect.top;
		swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.Stereo = false;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
		swapchainDesc.BufferCount = FrameBufferNum;
		swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		if (FAILED(factory->CreateSwapChainForHwnd(queue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain)))
			throw "";

		factory->Release();

		return { swapChain };
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::CopyTexture(Device* device,ResourceBase* srcResource, ResourceBase* dstResource)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		D3D12_TEXTURE_COPY_LOCATION dstLocation{};

		const auto dstDesc = dstResource->Get()->GetDesc();
		const auto width = dstDesc.Width;
		const auto height = dstDesc.Height;
		const auto uploadResourceRowPitch = srcResource->Get()->GetDesc().Width / height;

		srcLocation.pResource = srcResource->Get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			device->Get()->GetCopyableFootprints(&dstDesc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = width;
		srcLocation.PlacedFootprint.Footprint.Height = height;
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = uploadResourceRowPitch;
		srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		dstLocation.pResource = dstResource->Get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;

		list->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Execute()
	{
		queue->ExecuteCommandLists(1, (ID3D12CommandList**)(&list));
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Dispatch(std::uint32_t threadGroupCountX, std::uint32_t threadGroupCountY, std::uint32_t threadGroupCountZ)
	{
		list->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Fence(std::uint64_t index)
	{
		fenceValue[index]++;
		queue->Signal(fence[index], fenceValue[index]);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Wait(std::uint64_t index)
	{
		if (fence[index]->GetCompletedValue() < fenceValue[index])
		{
			fence[index]->SetEventOnCompletion(fenceValue[index], fenceEventHandle);
			WaitForSingleObject(fenceEventHandle, INFINITE);
		}
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::WaitAll(Device* device)
	{
		ID3D12Fence* fence = nullptr;
		device->Get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		
		constexpr std::uint64_t expectValue = 1;

		queue->Signal(fence, expectValue);
		if (fence->GetCompletedValue() != expectValue)
		{
			fence->SetEventOnCompletion(expectValue, fenceEventHandle);
			WaitForSingleObject(fenceEventHandle, INFINITE);
		}

		fence->Release();
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Reset(std::size_t index)
	{
		allocator[index]->Reset();
		list->Reset(allocator[index], nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void DX12::Command<FrameLatencyNum>::Close()
	{
		list->Close();
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetPipelineState(PipelineState* ps)
	{
		list->SetPipelineState(ps->Get());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle)
	{
		list->OMSetRenderTargets(1, &renderTargetHandle, false, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle)
	{
		list->OMSetRenderTargets(1, &renderTargetHandle, false, &depthStencilHandle);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetRenderTarget(std::uint32_t renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget)
	{
		list->OMSetRenderTargets(renderTagetHandleNum, renderTarget, false, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetRenderTarget(std::uint32_t renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle)
	{
		list->OMSetRenderTargets(renderTagetHandleNum, renderTarget, false, &depthStencilHandle);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetViewport(const D3D12_VIEWPORT& viewport)
	{
		list->RSSetViewports(1, &viewport);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetViewport(std::uint32_t num, D3D12_VIEWPORT* viewportPtr)
	{
		list->RSSetViewports(num, viewportPtr);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetScissorRect(const D3D12_RECT& rect)
	{
		list->RSSetScissorRects(1, &rect);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetScissorRect(std::uint32_t num, D3D12_RECT* rectPtr)
	{
		list->RSSetScissorRects(num, rectPtr);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetVertexBuffer(VertexBuffer* vbr)
	{
		list->IASetVertexBuffers(0, 1, &vbr->GetView());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetIndexBuffer(IndexBuffer* ibr)
	{
		list->IASetIndexBuffer(&ibr->GetView());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetGraphicsRootSignature(RootSignature* rootSignature)
	{
		list->SetGraphicsRootSignature(rootSignature->Get());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetComputeRootSignature(RootSignature* rootSignature)
	{
		list->SetComputeRootSignature(rootSignature->Get());
	}

	template<std::size_t FrameLatencyNum>
	template<typename T>
	inline void Command<FrameLatencyNum>::SetDescriptorHeap(DescriptorHeap<T>* dh)
	{
		list->SetDescriptorHeaps(1, &dh->Get());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetGraphicsRootDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		list->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetComputeRootDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		list->SetComputeRootDescriptorTable(index, gpuHandle);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
	{
		list->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(primitiveTopology));
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::DrawInstanced(std::uint32_t vertexNumPerInstance, std::uint32_t instanceNum)
	{
		list->DrawInstanced(vertexNumPerInstance, instanceNum, 0, 0);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::DrawIndexedInstanced(std::uint32_t indexNumPerInstance, std::uint32_t instanceNum)
	{
		list->DrawIndexedInstanced(indexNumPerInstance, instanceNum, 0, 0, 0);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Barrior(ResourceBase* rb, ResourceState rs)
	{
		if (rb->GetState() == rs)
			return;

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = rb->Get();
		barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(rb->GetState());
		barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(rs);
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->ResourceBarrier(1, &barrier);

		rb->SetState(rs);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::array<float, 4>& color)
	{
		list->ClearRenderTargetView(handle, color.data(), 0, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::ClearDepthView(D3D12_CPU_DESCRIPTOR_HANDLE handle, float d)
	{
		list->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH, d, 0, 0, nullptr);
	}




}