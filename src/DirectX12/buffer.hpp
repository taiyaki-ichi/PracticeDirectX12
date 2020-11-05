#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<iostream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace graphics
{


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


}