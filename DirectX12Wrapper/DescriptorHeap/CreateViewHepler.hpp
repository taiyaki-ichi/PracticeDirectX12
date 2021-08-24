#pragma once
#include"../Device.hpp"

#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	//DescriptorHeapのポインタを作成する関数
	template<typename DescriptorHeapTypeTag>
	inline ID3D12DescriptorHeap* GetDescriptorHeapPtr(Device*, std::uint32_t size) {
		static_assert(false);
	}
	
	//Handleのインクリメントのサイズ取得
	template<typename DescriptorHeapTypeTag>
	inline std::uint32_t GetDescriptorHandleIncrementSize(Device*) {
		static_assert(false);
	}
	
	//ビューを作成する関数
	template<typename DescriptorHeapTypeTag, typename ViewTypeTag>
	inline bool CreateView(Device*, ID3D12Resource*, const D3D12_CPU_DESCRIPTOR_HANDLE&) {
		static_assert(false);
	}


	//
	//
	//

	namespace DescriptorHeapTypeTag {
		struct CBV_SRV_UAV;
		struct DSV;
		struct RTV;
	}
	namespace ViewTypeTag {
		struct ConstantBuffer;
		struct DepthStencilBuffer;
		struct ShaderResource;
		struct CubeMap;
		struct UnorderedAccessResource;
	}

	template<>
	inline ID3D12DescriptorHeap* GetDescriptorHeapPtr<DescriptorHeapTypeTag::CBV_SRV_UAV>(Device* device, std::uint32_t size) 
	{
		ID3D12DescriptorHeap* result = nullptr;

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;
		desc.NumDescriptors = size;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		if (FAILED(device->Get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&result))))
			throw "";
		
		return result;
	}

	template<>
	inline ID3D12DescriptorHeap* GetDescriptorHeapPtr<DescriptorHeapTypeTag::DSV>(Device* device, std::uint32_t size)
	{
		ID3D12DescriptorHeap* result = nullptr;

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NodeMask = 0;
		desc.NumDescriptors = size;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		if (FAILED(device->Get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&result))))
			throw "";

		return result;
	}

	template<>
	inline ID3D12DescriptorHeap* GetDescriptorHeapPtr<DescriptorHeapTypeTag::RTV>(Device* device, std::uint32_t size)
	{
		ID3D12DescriptorHeap* result = nullptr;

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NodeMask = 0;
		desc.NumDescriptors = size;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if (FAILED(device->Get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&result))))
			throw "";

		return result;
	}

	template<>
	inline std::uint32_t GetDescriptorHandleIncrementSize<DescriptorHeapTypeTag::CBV_SRV_UAV>(Device* device) 
	{
		return device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	template<>
	inline std::uint32_t GetDescriptorHandleIncrementSize<DescriptorHeapTypeTag::DSV>(Device* device)
	{
		return device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	template<>
	inline std::uint32_t GetDescriptorHandleIncrementSize<DescriptorHeapTypeTag::RTV>(Device* device)
	{
		return device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, ViewTypeTag::ConstantBuffer>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
		cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(resource->GetDesc().Width);


		device->Get()->CreateConstantBufferView(&cbvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool  CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, ViewTypeTag::ShaderResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		const auto& desc = resource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = desc.Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (desc.DepthOrArraySize > 1){
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
			srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
		}
		else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
		}

		device->Get()->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool  CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, ViewTypeTag::DepthStencilBuffer>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		const auto& desc = resource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (desc.DepthOrArraySize > 1)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
			srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
		}
		
		device->Get()->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, ViewTypeTag::CubeMap>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle) 
	{
		const auto& desc = resource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = desc.Format;
		srvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

		device->Get()->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, ViewTypeTag::UnorderedAccessResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		const auto& desc = resource->GetDesc();
		D3D12_UNORDERED_ACCESS_VIEW_DESC resDesc{};
		resDesc.Format = desc.Format;
		resDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		resDesc.Texture2D.MipSlice = 0;
		resDesc.Texture2D.PlaneSlice = 0;

		device->Get()->CreateUnorderedAccessView(resource, nullptr, &resDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool  CreateView<DescriptorHeapTypeTag::DSV, ViewTypeTag::DepthStencilBuffer>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		const auto& desc = resource->GetDesc();
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
		if (desc.DepthOrArraySize > 1)
		{
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		}
		else{
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		}
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		device->Get()->CreateDepthStencilView(resource, &dsvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::RTV, ViewTypeTag::ShaderResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		const auto& desc = resource->GetDesc();
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = desc.Format;
		if (desc.DepthOrArraySize > 1)
		{
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		}
		else {
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		}

		device->Get()->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

		return true;
	}
}