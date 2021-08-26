#pragma once
#include"Format.hpp"
#include<utility>
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	//DepthStencil‚ÌDescriptorHeap‚ÉView‚ğì‚é‚Ég—p
	inline constexpr std::optional<DXGI_FORMAT> get_depth_stencil_view_format(component_type componentType, std::uint8_t componentSize, std::uint8_t componentNum) {
		if (componentType == component_type::UNSIGNED_NORMALIZE_FLOAT && componentSize == 16 && componentNum == 1)
			return DXGI_FORMAT_D16_UNORM;
		if (componentType == component_type::FLOAT && componentSize == 32 && componentNum == 1)
			return DXGI_FORMAT_D32_FLOAT;
		return std::nullopt;
	}


	inline void create_CBV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, std::uint32_t sizeInBytes)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
		desc.BufferLocation = resource->GetGPUVirtualAddress();
		desc.SizeInBytes = sizeInBytes;
		device->CreateConstantBufferView(&desc, cpuHandle);
	}

	template<component_type ViewComponentType, typename typeless_format>
	inline void create_texture2D_SRV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSline, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = get_dxgi_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = mipLevels;
		desc.Texture2D.MostDetailedMip = mostDetailedMip;
		desc.Texture2D.PlaneSlice = planeSline;
		desc.Texture2D.ResourceMinLODClamp = resourceMinLODClamp;
		device->CreateShaderResourceView(resource, &desc, cpuHandle);
	}

	template<component_type ViewComponentType, typename typeless_format>
	inline void create_texture2D_array_SRV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = get_dxgi_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipLevels = mipLevels;
		desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
		desc.Texture2DArray.PlaneSlice = planeSlice;
		desc.Texture2DArray.ResourceMinLODClamp = resourceMinLODClamp;
		device->CreateShaderResourceView(resource, &desc, cpuHandle);
	}

	template<component_type ViewComponentType, typename typeless_format>
	inline void create_texture_cube_SRV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = get_dxgi_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipLevels = mipLevels;
		desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
		desc.Texture2DArray.PlaneSlice = planeSlice;
		desc.Texture2DArray.ResourceMinLODClamp = resourceMinLODClamp;
		device->CreateShaderResourceView(resource, &desc, cpuHandle);
	}

	template<component_type ViewComponentType, typename typeless_format>
	inline void create_texture2D_UAV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		ID3D12Resource* counterResource, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Format = get_dxgi_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;
		desc.Texture2D.PlaneSlice = planeSlice;
		device->CreateUnorderedAccessView(resource, counterResource, &desc, cpuHandle);
	}

	template<component_type ViewComponentType, typename typeless_format>
	inline void create_texture2D_array_UAV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		ID3D12Resource* counterResource, std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Format = get_dxgi_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipSlice = mipSlice;
		desc.Texture2DArray.PlaneSlice = planeSlice;
		device->CreateUnorderedAccessView(resource, counterResource, &desc, cpuHandle);
	}

	template<component_type ViewComponentType, typename typeless_format>
	inline void create_texture2D_DSV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t mipSlice)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Format = get_depth_stencil_view_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;
		device->CreateDepthStencilView(resource, &desc, cpuHandle);
	}

	template<component_type ViewComponentType,typename typeless_format>
	inline void create_texture2D_array_DSV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Format = get_depth_stencil_view_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipSlice = mipSlice;
		device->CreateDepthStencilView(resource, &desc, cpuHandle);
	}

	template<component_type ViewComponentType, typename typeless_format>
	inline void create_texture2D_RTV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = get_dxgi_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;
		desc.Texture2D.PlaneSlice = planeSlice;
		device->CreateRenderTargetView(resource, &desc, cpuHandle);
	}

	template<component_type ViewComponentType, typename typeless_format>
	inline void create_texture2D_array_RTV(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = get_dxgi_format(ViewComponentType, typeless_format::component_size, typeless_format::component_num).value();
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipSlice = mipSlice;
		desc.Texture2DArray.PlaneSlice = planeSlice;
		device->CreateRenderTargetView(resource, &desc, cpuHandle);
	}


}