#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<DirectXTex.h>
#include<iostream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")

namespace graphics
{
	size_t alignment_size(size_t size, size_t alignment) {
		return size + alignment - size % alignment;
	}

	//頂点バッファとインデックスバッファの生成し使用、sizeはマップする予定の配列の大きさ
	ID3D12Resource* create_buffer(ID3D12Device* device,size_t size) {

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resdesc.Width = size;
		resdesc.Height = 1;
		resdesc.DepthOrArraySize = 1;
		resdesc.MipLevels = 1;
		resdesc.Format = DXGI_FORMAT_UNKNOWN;
		resdesc.SampleDesc.Count = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		ID3D12Resource* vertBuff = nullptr;
		//成功
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertBuff)))) {

			return vertBuff;
		}
		//失敗
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}


	//バッファに情報をマップ
	template<typename Value,size_t N>
	bool map(ID3D12Resource* buffer, const Value (&a)[N])
	{
		Value* target = nullptr;
		auto result = buffer->Map(0, nullptr, (void**)&target);
		//失敗したとき
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(a), std::end(a), target);
		buffer->Unmap(0, nullptr);

		return true;
	}

	//頂点バッファビューの取得
	D3D12_VERTEX_BUFFER_VIEW get_vertex_buffer_view(ID3D12Resource* buffer, size_t size, size_t stride)
	{
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = buffer->GetGPUVirtualAddress();//バッファの仮想アドレス
		view.SizeInBytes = size;//全バイト数
		view.StrideInBytes = stride;//1頂点あたりのバイト数
		return view;
	}

	//インデックスバッファビューの生成、strideは符号なし整数で固定
	D3D12_INDEX_BUFFER_VIEW get_index_buffer_view(ID3D12Resource* buffer, size_t size)
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = buffer->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R16_UINT;
		view.SizeInBytes = size;
		return view;
	}

	//画像アップロード用のバッファの生成
	ID3D12Resource* create_texture_unload_buffer(ID3D12Device* device,const DirectX::Image* image)
	{
		
		D3D12_HEAP_PROPERTIES heapprop{};

		//マップ可能にする
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;

		//アップロード用なのでUNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//単一アダプターのためそれぞれ0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		
		D3D12_RESOURCE_DESC resdesc{};

		//単なるデータ
		resdesc.Format = DXGI_FORMAT_UNKNOWN;
		//単なるバッファ
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

		//データサイズ
		resdesc.Width = static_cast<UINT64>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) * image->height;
		resdesc.Height = 1;
		resdesc.DepthOrArraySize = 1;
		resdesc.MipLevels = 1;

		//連続したデータ
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		//特になし
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		//普通のテクスチャなのでアンチエイリアシングしない
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;

		ID3D12Resource* uploadBuff = nullptr;
		//成功
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuff)))) {

			return uploadBuff;
		}
		//失敗
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}


	//テクスチャ用のバッファ、コピーされる側
	ID3D12Resource* create_texture_buffer(ID3D12Device* device, const DirectX::TexMetadata& metaData)
	{
		D3D12_HEAP_PROPERTIES heapprop{};

		//テクスチャ用
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;

		//こっちもUNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//単一アダプターのためそれぞれ0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		//ほとんどmetaDataから
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Format = metaData.format;
		resdesc.Width = static_cast<UINT>(metaData.width);//幅
		resdesc.Height = static_cast<UINT>(metaData.height);//高さ
		resdesc.DepthOrArraySize = static_cast<UINT16>(metaData.arraySize);//2Dで配列でもないので１
		resdesc.MipLevels = static_cast<UINT16>(metaData.mipLevels);//ミップマップしないのでミップ数は１つ
		resdesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData.dimension);//2Dテクスチャ用
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		//特になし
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		//普通のテクスチャなのでアンチエイリアシングしない
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;

		ID3D12Resource* textureBuff = nullptr;
		//成功
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_COPY_DEST,//コピー先なので
			nullptr,
			IID_PPV_ARGS(&textureBuff)))) {

			return textureBuff;
		}
		//失敗
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}

	namespace {


		//画像のmapの中で使用、仮
		//GetCopyableFootprintsでできるならそれがいい
		void copy_image(uint8_t* mapForImage,const DirectX::Image* image) {
			uint8_t* srcAddress = image->pixels;
			size_t rowPitch = alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

			for (int y = 0; y < image->height; ++y) {
				std::copy_n(srcAddress,
					rowPitch,
					mapForImage);//コピー
				//1行ごとの辻褄を合わせてやる
				srcAddress += image->rowPitch;
				mapForImage += rowPitch;
			}
		}
	}

	//画像用、とりあえずサンプルどうりに
	bool map(ID3D12Resource* buffer, const DirectX::Image* image)
	{
		uint8_t* mapforImage = nullptr;
		if (FAILED(buffer->Map(0, nullptr, (void**)&mapforImage))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}
		
		copy_image(mapforImage, image);

		buffer->Unmap(0, nullptr);
	}


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


	void copy_texture(ID3D12CommandAllocator* allocator,ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* queue,
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