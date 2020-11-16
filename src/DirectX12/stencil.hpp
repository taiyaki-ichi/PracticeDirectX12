#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<iostream>
#include"descriptor_heap.hpp"
#include<array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace graphics
{


	//ステンシルバッファの生成
	ID3D12Resource* create_stencil_buffer(ID3D12Device* device ,UINT64 windowWidth,UINT64 windowHeight)
	{
		D3D12_RESOURCE_DESC desc{};
		//テクスチャデータ
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = windowWidth;
		desc.Height = windowHeight;
		desc.DepthOrArraySize = 1;
		//
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.SampleDesc.Count = 1;
		//でぃぷすステンシルとして使用
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES prop{};
		//typeはデフォルト、ほかunknow
		prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//重要らしい
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.DepthStencil.Depth = 1.f;
		clearValue.DepthStencil.Stencil = 0;
		//
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;


		ID3D12Resource* buffer = nullptr;
		auto result = device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			//逐一変更する
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			IID_PPV_ARGS(&buffer)
		);

		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}

		return buffer;
	}


	//ステンシルバッファ用
	ID3D12DescriptorHeap* create_stencil_descriptor_heap(ID3D12Device* device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ID3D12DescriptorHeap* dh = nullptr;
		if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dh)))) {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}

		return dh;
	}

	//ビューの生成
	void set_stencil_buffer_view(ID3D12Device* device, ID3D12DescriptorHeap* heap, ID3D12Resource* buffer)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		//
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(
			buffer,
			&desc,
			heap->GetCPUDescriptorHandleForHeapStart()
		);
	}




}