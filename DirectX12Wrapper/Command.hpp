#pragma once
#include"device.hpp"
#include"swap_chain.hpp"
#include"pipeline_state.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include"resource/index_buffer_resource.hpp"
#include"descriptor_heap.hpp"
#include<array>
#include<algorithm>
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	template<std::size_t FrameLatencyNum>
	class command
	{
		release_unique_ptr<ID3D12CommandQueue> queue_ptr{};
		release_unique_ptr<ID3D12GraphicsCommandList> list_ptr{};
				
		std::array<release_unique_ptr<ID3D12CommandAllocator>, FrameLatencyNum> allocator_ptrs{};
		std::array<release_unique_ptr<ID3D12Fence>, FrameLatencyNum> fence_ptrs{};
		std::array<std::uint64_t, FrameLatencyNum> fenceValue{};
		HANDLE fenceEventHandle = nullptr;

	public:
		command() = default;
		~command() = default;

		command(command<FrameLatencyNum>&&) = default;
		command& operator=(command<FrameLatencyNum>&&) = default;

		void initialize(device&);

		template<typename SrcResource,typename DstResource>
		void copy_texture(device&,SrcResource& srcResource, DstResource& dstResource);

		void execute();
		void dispatch(std::uint32_t threadGroupCountX, std::uint32_t threadGroupCountY, std::uint32_t threadGroupCountZ);

		//index番目のfenceValueを更新し、Queueにその値をSignalする
		void fence(std::uint64_t index);

		//index番目のコマンドの終了を待つ
		void wait(std::uint64_t index);
		//現在発行されている全てのコマンドの処理を待つ
		void wait_all(device&);

		//indexはアロケータのインデックス
		void reset(std::size_t index);

		void close();

		template<typename VertexLayout,typename RenderTargetFormats>
		void set_pipeline_state(graphics_pipeline_state<VertexLayout,RenderTargetFormats>&);
		void set_pipeline_state(compute_pipeline_state&);

		//OMSetRenderTargetsの最適化について、どのターゲットのViewもおなじディスクリプタヒープ連続して生成されていないとみなしている
		void set_render_target(std::optional<std::initializer_list<D3D12_CPU_DESCRIPTOR_HANDLE>> renderTargetHandle,
			std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> depthStencilHandle = std::nullopt);

		void set_viewport(const D3D12_VIEWPORT& viewport);
		void set_viewport(std::initializer_list<D3D12_VIEWPORT>);

		void set_scissor_rect(const D3D12_RECT& rect);
		void set_scissor_rect(std::initializer_list<D3D12_RECT>);

		template<typename... Formats>
		void set_vertex_buffer(vertex_buffer_resource<Formats...>&);

		template<typename Format>
		void set_index_buffer(index_buffer_resource<Format>&);

		void set_graphics_root_signature(root_signature&);
		void set_compute_root_signature(root_signature&);

		template<typename DescriptorHeap>
		void set_descriptor_heap(DescriptorHeap&);
		void set_graphics_root_descriptor_table(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE);
		void set_compute_root_descriptor_table(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE);

		void set_primitive_topology(primitive_topology);

		void draw_instanced(std::uint32_t vertexNumPerInstance, std::uint32_t instanceNum = 1);
		void draw_indexed_instanced(std::uint32_t indexNumPerInstance, std::uint32_t instanceNum = 1);

		template<typename Resource>
		void barrior(Resource&, resource_state);

		void clear_render_target_view(D3D12_CPU_DESCRIPTOR_HANDLE, const std::array<float, 4>&);
		void clear_depth_view(D3D12_CPU_DESCRIPTOR_HANDLE, float);

		ID3D12CommandQueue* get_queue() noexcept;
		ID3D12GraphicsCommandList* get_list() noexcept;
		ID3D12CommandAllocator* get_allocator(std::size_t index);
		ID3D12Fence* get_fence(std::size_t index);
	};


	//
	//
	//


	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::initialize(device& device)
	{
		for (std::size_t i = 0; i < allocator_ptrs.size(); i++)
		{
			ID3D12CommandAllocator* tmp = nullptr;
			if (FAILED(device.get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&tmp))))
				THROW_EXCEPTION("");
			allocator_ptrs[i].reset(tmp);
		}


		{
			ID3D12GraphicsCommandList* tmp = nullptr;
			if (FAILED(device.get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator_ptrs[0].get(), nullptr, IID_PPV_ARGS(&tmp))))
				THROW_EXCEPTION("");
			list_ptr.reset(tmp);
		}


		{
			ID3D12CommandQueue* tmp = nullptr;
			D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
			cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//タイムアウトナシ
			cmdQueueDesc.NodeMask = 0;
			cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//プライオリティ特に指定なし
			cmdQueueDesc.Type = list_ptr->GetType();			//ここはコマンドリストと合わせる
			if (FAILED(device.get()->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&tmp))))
				THROW_EXCEPTION("");
			queue_ptr.reset(tmp);
		}

		//全て１で
		std::fill(fenceValue.begin(), fenceValue.end(), 1);

		for (std::size_t i = 0; i < FrameLatencyNum; i++)
		{
			ID3D12Fence* tmp = nullptr;
			if (FAILED(device.get()->CreateFence(fenceValue[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tmp))))
				THROW_EXCEPTION("");
			fence_ptrs[i].reset(tmp);
		}

		fenceEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);

		close();//必要
	}


	template<std::size_t FrameLatencyNum>
	template<typename SrcResource, typename DstResource>
	inline void command<FrameLatencyNum>::copy_texture(device& device, SrcResource& srcResource, DstResource& dstResource)
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
			device.get()->GetCopyableFootprints(&dstDesc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = width;
		srcLocation.PlacedFootprint.Footprint.Height = height;
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = uploadResourceRowPitch;
		srcLocation.PlacedFootprint.Footprint.Format = dstDesc.Format;

		dstLocation.pResource = dstResource.get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;

		list_ptr->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::execute()
	{
		queue_ptr->ExecuteCommandLists(1, (ID3D12CommandList**)(&list_ptr));
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::dispatch(std::uint32_t threadGroupCountX, std::uint32_t threadGroupCountY, std::uint32_t threadGroupCountZ)
	{
		list_ptr->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::fence(std::uint64_t index)
	{
		fenceValue[index]++;
		queue_ptr->Signal(fence_ptrs[index].get(), fenceValue[index]);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::wait(std::uint64_t index)
	{
		if (fence_ptrs[index]->GetCompletedValue() < fenceValue[index])
		{
			fence_ptrs[index]->SetEventOnCompletion(fenceValue[index], fenceEventHandle);
			WaitForSingleObject(fenceEventHandle, INFINITE);
		}
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::wait_all(device& device)
	{
		ID3D12Fence* fence = nullptr;
		device.get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		
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
	inline void command<FrameLatencyNum>::reset(std::size_t index)
	{
		allocator_ptrs[index]->Reset();
		list_ptr->Reset(allocator_ptrs[index].get(), nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void DX12::command<FrameLatencyNum>::close()
	{
		list_ptr->Close();
	}

	template<std::size_t FrameLatencyNum>
	template<typename VertexLayout, typename RenderTargetFormats>
	inline void command<FrameLatencyNum>::set_pipeline_state(graphics_pipeline_state<VertexLayout,RenderTargetFormats>& ps)
	{
		list_ptr->SetPipelineState(ps.get());
	}

	template<std::size_t FrameLatencyNum>
	inline void DX12::command<FrameLatencyNum>::set_pipeline_state(compute_pipeline_state& ps)
	{
		list_ptr->SetPipelineState(ps.Get());
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_render_target(std::optional<std::initializer_list<D3D12_CPU_DESCRIPTOR_HANDLE>> renderTargetHandle,
		std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> depthStencilHandle)
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* rth = (renderTargetHandle && renderTargetHandle.value().size() > 0) ? renderTargetHandle.value().begin() : nullptr;
		std::size_t rthNum = renderTargetHandle ? renderTargetHandle.value().size() : 0;
		const D3D12_CPU_DESCRIPTOR_HANDLE* dsh = depthStencilHandle ? (&depthStencilHandle.value()) : nullptr;
		list_ptr->OMSetRenderTargets(rthNum, rth, false, dsh);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_viewport(const D3D12_VIEWPORT& viewport)
	{
		list_ptr->RSSetViewports(1, &viewport);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_viewport(std::initializer_list<D3D12_VIEWPORT> list)
	{
		list_ptr->RSSetViewports(list.size(), list.begin());
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_scissor_rect(const D3D12_RECT& rect)
	{
		list_ptr->RSSetScissorRects(1, &rect);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_scissor_rect(std::initializer_list<D3D12_RECT> list)
	{
		list_ptr->RSSetScissorRects(list.size(), list.begin());
	}

	template<std::size_t FrameLatencyNum>
	template<typename... Formats>
	inline void command<FrameLatencyNum>::set_vertex_buffer(vertex_buffer_resource<Formats...>& vbr)
	{
		list_ptr->IASetVertexBuffers(0, 1, &vbr.get_view());
	}

	template<std::size_t FrameLatencyNum>
	template<typename Format>
	inline void command<FrameLatencyNum>::set_index_buffer(index_buffer_resource<Format>& ibr)
	{
		list_ptr->IASetIndexBuffer(&ibr.get_view());
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_graphics_root_signature(root_signature& rootSignature)
	{
		list_ptr->SetGraphicsRootSignature(rootSignature.Get());
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_compute_root_signature(root_signature& rootSignature)
	{
		list_ptr->SetComputeRootSignature(rootSignature.Get());
	}

	template<std::size_t FrameLatencyNum>
	template<typename DescriptorHeap>
	inline void command<FrameLatencyNum>::set_descriptor_heap(DescriptorHeap& dh)
	{
		ID3D12DescriptorHeap* tmp[] = { dh.get() };
		list_ptr->SetDescriptorHeaps(1, tmp);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_graphics_root_descriptor_table(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		list_ptr->SetGraphicsRootDescriptorTable(index, gpuHandle);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_compute_root_descriptor_table(std::uint32_t index, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		list_ptr->SetComputeRootDescriptorTable(index, gpuHandle);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::set_primitive_topology(primitive_topology primitiveTopology)
	{
		list_ptr->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(primitiveTopology));
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::draw_instanced(std::uint32_t vertexNumPerInstance, std::uint32_t instanceNum)
	{
		list_ptr->DrawInstanced(vertexNumPerInstance, instanceNum, 0, 0);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::draw_indexed_instanced(std::uint32_t indexNumPerInstance, std::uint32_t instanceNum)
	{
		list_ptr->DrawIndexedInstanced(indexNumPerInstance, instanceNum, 0, 0, 0);
	}

	template<std::size_t FrameLatencyNum>
	template<typename Resource>
	inline void command<FrameLatencyNum>::barrior(Resource& r, resource_state s)
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
	inline void command<FrameLatencyNum>::clear_render_target_view(D3D12_CPU_DESCRIPTOR_HANDLE handle, const std::array<float, 4>& color)
	{
		list_ptr->ClearRenderTargetView(handle, color.data(), 0, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline void command<FrameLatencyNum>::clear_depth_view(D3D12_CPU_DESCRIPTOR_HANDLE handle, float d)
	{
		list_ptr->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH, d, 0, 0, nullptr);
	}

	template<std::size_t FrameLatencyNum>
	inline ID3D12CommandQueue* command<FrameLatencyNum>::get_queue() noexcept
	{
		return queue_ptr.get();
	}

	template<std::size_t FrameLatencyNum>
	inline ID3D12GraphicsCommandList* command<FrameLatencyNum>::get_list() noexcept
	{
		return list_ptr.get();
	}

	template<std::size_t FrameLatencyNum>
	inline ID3D12CommandAllocator* command<FrameLatencyNum>::get_allocator(std::size_t index)
	{
		return allocator_ptrs[index].get();
	}

	template<std::size_t FrameLatencyNum>
	inline ID3D12Fence* command<FrameLatencyNum>::get_fence(std::size_t index)
	{
		return fence_ptrs[index].get();
	}
}