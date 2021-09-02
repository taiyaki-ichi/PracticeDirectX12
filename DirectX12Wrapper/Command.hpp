#pragma once
#include"Device.hpp"
#include"SwapChain.hpp"
#include"PipelineState.hpp"
#include"Resource/vertex_buffer_resource.hpp"
#include"Resource/index_buffer_resource.hpp"
#include"DescriptorHeap.hpp"
#include<array>
#include<algorithm>
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	template<std::size_t FrameLatencyNum = 2>
	class Command
	{
		release_unique_ptr<ID3D12CommandQueue> queue_ptr{};
		release_unique_ptr<ID3D12GraphicsCommandList> list_ptr{};
				
		std::array<release_unique_ptr<ID3D12CommandAllocator>, FrameLatencyNum> allocator_ptrs{};
		std::array<release_unique_ptr<ID3D12Fence>, FrameLatencyNum> fence_ptrs{};
		std::array<std::uint64_t, FrameLatencyNum> fenceValue{};
		HANDLE fenceEventHandle = nullptr;

	public:
		Command() = default;
		~Command() = default;

		Command(Command<FrameLatencyNum>&&) = default;
		Command& operator=(Command<FrameLatencyNum>&&) = default;

		void Initialize(Device&);

		template<typename SrcResource,typename DstResource>
		void CopyTexture(Device&,SrcResource& srcResource, DstResource& dstResource);

		void Execute();
		void Dispatch(std::uint32_t threadGroupCountX, std::uint32_t threadGroupCountY, std::uint32_t threadGroupCountZ);

		//index番目のfenceValueを更新し、Queueにその値をSignalする
		void Fence(std::uint64_t index);

		//index番目のコマンドの終了を待つ
		void Wait(std::uint64_t index);
		//現在発行されている全てのコマンドの処理を待つ
		//いらないかも
		void WaitAll(Device&);

		//indexはアロケータのインデックス
		void Reset(std::size_t index);

		void Close();

		template<typename VertexLayout,typename RenderTargetFormats>
		void SetPipelineState(graphics_pipeline_state<VertexLayout,RenderTargetFormats>&);
		void SetPipelineState(compute_pipeline_state&);

		//OMSetRenderTargetsの最適化について、どのターゲットのViewもおなじディスクリプタヒープ連続して生成されていないとみなしている
		void SetRenderTarget(std::optional<std::initializer_list<D3D12_CPU_DESCRIPTOR_HANDLE>> renderTargetHandle,
			std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> depthStencilHandle = std::nullopt);

		void SetViewport(const D3D12_VIEWPORT& viewport);
		void SetViewport(std::initializer_list<D3D12_VIEWPORT>);

		void SetScissorRect(const D3D12_RECT& rect);
		void SetScissorRect(std::initializer_list<D3D12_RECT>);

		template<typename... Formats>
		void SetVertexBuffer(vertex_buffer_resource<Formats...>&);

		template<typename Format>
		void SetIndexBuffer(index_buffer_resource<Format>&);

		void SetGraphicsRootSignature(RootSignature&);
		void SetComputeRootSignature(RootSignature&);

		template<typename DescriptorHeap>
		void SetDescriptorHeap(DescriptorHeap&);
		void SetGraphicsRootDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE);
		void SetComputeRootDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE);

		void SetPrimitiveTopology(PrimitiveTopology);

		void DrawInstanced(std::uint32_t vertexNumPerInstance, std::uint32_t instanceNum = 1);
		void DrawIndexedInstanced(std::uint32_t indexNumPerInstance, std::uint32_t instanceNum = 1);

		template<typename Resource>
		void Barrior(Resource&, resource_state);

		void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE, const std::array<float, 4>&);
		void ClearDepthView(D3D12_CPU_DESCRIPTOR_HANDLE, float);

		ID3D12CommandQueue* get_queue() noexcept;
	};


	//
	//
	//


	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Initialize(Device& device)
	{
		for (std::size_t i = 0; i < allocator_ptrs.size(); i++)
		{
			ID3D12CommandAllocator* tmp = nullptr;
			if (FAILED(device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&tmp))))
				throw "";
			allocator_ptrs[i].reset(tmp);
		}


		{
			ID3D12GraphicsCommandList* tmp = nullptr;
			if (FAILED(device.Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator_ptrs[0].get(), nullptr, IID_PPV_ARGS(&tmp))))
				throw "";
			list_ptr.reset(tmp);
		}


		{
			ID3D12CommandQueue* tmp = nullptr;
			D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
			cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//タイムアウトナシ
			cmdQueueDesc.NodeMask = 0;
			cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//プライオリティ特に指定なし
			cmdQueueDesc.Type = list_ptr->GetType();			//ここはコマンドリストと合わせる
			if (FAILED(device.Get()->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&tmp))))
				throw "";
			queue_ptr.reset(tmp);
		}

		//全て１で
		std::fill(fenceValue.begin(), fenceValue.end(), 1);

		for (std::size_t i = 0; i < FrameLatencyNum; i++)
		{
			ID3D12Fence* tmp = nullptr;
			if (FAILED(device.Get()->CreateFence(fenceValue[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tmp))))
				throw "";
			fence_ptrs[i].reset(tmp);
		}

		fenceEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);

		Close();//必要
	}


	template<std::size_t FrameLatencyNum>
	template<typename SrcResource, typename DstResource>
	inline void Command<FrameLatencyNum>::CopyTexture(Device& device, SrcResource& srcResource, DstResource& dstResource)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		D3D12_TEXTURE_COPY_LOCATION dstLocation{};

		const auto dstDesc = dstResource.get()->GetDesc();
		const auto width = dstDesc.Width;
		const auto height = dstDesc.Height;
		const auto uploadResourceRowPitch = srcResource.get()->GetDesc().Width / height;

		srcLocation.pResource = srcResource.get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			device.Get()->GetCopyableFootprints(&dstDesc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = width;
		srcLocation.PlacedFootprint.Footprint.Height = height;
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = uploadResourceRowPitch;
		srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		dstLocation.pResource = dstResource.get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;

		list_ptr->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Execute()
	{
		queue_ptr->ExecuteCommandLists(1, (ID3D12CommandList**)(&list_ptr));
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Dispatch(std::uint32_t threadGroupCountX, std::uint32_t threadGroupCountY, std::uint32_t threadGroupCountZ)
	{
		list_ptr->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Fence(std::uint64_t index)
	{
		fenceValue[index]++;
		queue_ptr->Signal(fence_ptrs[index].get(), fenceValue[index]);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::Wait(std::uint64_t index)
	{
		if (fence_ptrs[index]->GetCompletedValue() < fenceValue[index])
		{
			fence_ptrs[index]->SetEventOnCompletion(fenceValue[index], fenceEventHandle);
			WaitForSingleObject(fenceEventHandle, INFINITE);
		}
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::WaitAll(Device& device)
	{
		ID3D12Fence* fence = nullptr;
		device.Get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		
		constexpr std::uint64_t expectValue = 1;

		queue_ptr->Signal(fence, expectValue);
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
		allocator_ptrs[index]->Reset();
		list_ptr->Reset(allocator_ptrs[index].get(), nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void DX12::Command<FrameLatencyNum>::Close()
	{
		list_ptr->Close();
	}

	template<std::size_t FrameLatencyNum>
	template<typename VertexLayout, typename RenderTargetFormats>
	inline void Command<FrameLatencyNum>::SetPipelineState(graphics_pipeline_state<VertexLayout,RenderTargetFormats>& ps)
	{
		list_ptr->SetPipelineState(ps.Get());
	}

	template<std::size_t FrameLatencyNum>
	inline void DX12::Command<FrameLatencyNum>::SetPipelineState(compute_pipeline_state& ps)
	{
		list_ptr->SetPipelineState(ps.Get());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetRenderTarget(std::optional<std::initializer_list<D3D12_CPU_DESCRIPTOR_HANDLE>> renderTargetHandle,
		std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> depthStencilHandle)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* rth = (renderTargetHandle && renderTargetHandle.value().size() > 0) ? renderTargetHandle.value().begin() : nullptr;
		std::size_t rthNum = renderTargetHandle ? renderTargetHandle.value().size() : 0;
		const D3D12_CPU_DESCRIPTOR_HANDLE* dsh = depthStencilHandle ? (&depthStencilHandle.value()) : nullptr;
		list_ptr->OMSetRenderTargets(rthNum, rth, false, dsh);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetViewport(const D3D12_VIEWPORT& viewport)
	{
		list_ptr->RSSetViewports(1, &viewport);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetViewport(std::initializer_list<D3D12_VIEWPORT> list)
	{
		list_ptr->RSSetViewports(list.size(), list.begin());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetScissorRect(const D3D12_RECT& rect)
	{
		list_ptr->RSSetScissorRects(1, &rect);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetScissorRect(std::initializer_list<D3D12_RECT> list)
	{
		list_ptr->RSSetScissorRects(list.size(), list.begin());
	}

	template<std::size_t FrameLatencyNum>
	template<typename... Formats>
	inline void Command<FrameLatencyNum>::SetVertexBuffer(vertex_buffer_resource<Formats...>& vbr)
	{
		list_ptr->IASetVertexBuffers(0, 1, &vbr.get_view());
	}

	template<std::size_t FrameLatencyNum>
	template<typename Format>
	inline void Command<FrameLatencyNum>::SetIndexBuffer(index_buffer_resource<Format>& ibr)
	{
		list_ptr->IASetIndexBuffer(&ibr.get_view());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetGraphicsRootSignature(RootSignature& rootSignature)
	{
		list_ptr->SetGraphicsRootSignature(rootSignature.Get());
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetComputeRootSignature(RootSignature& rootSignature)
	{
		list_ptr->SetComputeRootSignature(rootSignature.Get());
	}

	template<std::size_t FrameLatencyNum>
	template<typename DescriptorHeap>
	inline void Command<FrameLatencyNum>::SetDescriptorHeap(DescriptorHeap& dh)
	{
		ID3D12DescriptorHeap* tmp[] = { dh.get() };
		list_ptr->SetDescriptorHeaps(1, tmp);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetGraphicsRootDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		list_ptr->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetComputeRootDescriptorTable(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		list_ptr->SetComputeRootDescriptorTable(index, gpuHandle);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
	{
		list_ptr->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(primitiveTopology));
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::DrawInstanced(std::uint32_t vertexNumPerInstance, std::uint32_t instanceNum)
	{
		list_ptr->DrawInstanced(vertexNumPerInstance, instanceNum, 0, 0);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::DrawIndexedInstanced(std::uint32_t indexNumPerInstance, std::uint32_t instanceNum)
	{
		list_ptr->DrawIndexedInstanced(indexNumPerInstance, instanceNum, 0, 0, 0);
	}

	template<std::size_t FrameLatencyNum>
	template<typename Resource>
	inline void Command<FrameLatencyNum>::Barrior(Resource& r, resource_state s)
	{
		if (r.get_state() == s)
			return;

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = r.get();
		barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(r.get_state());
		barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(s);
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list_ptr->ResourceBarrier(1, &barrier);

		r.set_state(s);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::array<float, 4>& color)
	{
		list_ptr->ClearRenderTargetView(handle, color.data(), 0, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void Command<FrameLatencyNum>::ClearDepthView(D3D12_CPU_DESCRIPTOR_HANDLE handle, float d)
	{
		list_ptr->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH, d, 0, 0, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline ID3D12CommandQueue* Command<FrameLatencyNum>::get_queue() noexcept
	{
		return queue_ptr.get();
	}


}