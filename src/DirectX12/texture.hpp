#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<DirectXTex.h>
#include<iostream>
#include"utility.hpp"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")

namespace graphics
{


	//コピーに使用する構造体を2つ返す
	//アップロードされた方、コピーされる方、、の順
	std::pair<D3D12_TEXTURE_COPY_LOCATION, D3D12_TEXTURE_COPY_LOCATION>
		get_texture_copy_location(ID3D12Device* device, ID3D12Resource* uploadBuff, ID3D12Resource* textureBuff, const DirectX::TexMetadata& metaData, const DirectX::Image* image)
	{
		D3D12_TEXTURE_COPY_LOCATION src{};
		D3D12_TEXTURE_COPY_LOCATION dst{};

		dst.pResource = textureBuff;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		src.pResource = uploadBuff;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
		UINT nrow;
		UINT64 rowsize, size;
		auto desc = textureBuff->GetDesc();
		device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
		src.PlacedFootprint = footprint;
		src.PlacedFootprint.Offset = 0;
		src.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData.width);
		src.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData.height);
		src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData.depth);
		src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		src.PlacedFootprint.Footprint.Format = image->format;
		
		return { src,dst };
	}


	void copy_texture(ID3D12CommandAllocator* allocator, ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* queue,
		ID3D12Fence* fence, UINT64& fenceVal,
		D3D12_TEXTURE_COPY_LOCATION& uploadLocation, D3D12_TEXTURE_COPY_LOCATION& textureLocation)
	{

		//allocator->Reset();
		//commandList->Reset(allocator, nullptr);

		commandList->CopyTextureRegion(&textureLocation, 0, 0, 0, &uploadLocation, nullptr);

		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = textureLocation.pResource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		commandList->ResourceBarrier(1, &BarrierDesc);
		commandList->Close();

		//実行
		ID3D12CommandList* cmdlists[] = { commandList };
		queue->ExecuteCommandLists(1, cmdlists);
		//待ち

		queue->Signal(fence, ++fenceVal);
		//std::cout << UINT64_MAX << "\n";

		if (fence->GetCompletedValue() != fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		allocator->Reset();//キューをクリア
		commandList->Reset(allocator, nullptr);
	}



}