#pragma once
#include<utility>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace dev
{
	enum class ViewComponentType
	{
		Float,
		Uint,
		UnsignedNormalizedFloat,
	};

	enum class ViewDimention
	{
		Texture2D,
		Textre2DArray,
		TextureCube,
	};

	//ViewÇçÏê¨Ç∑ÇÈç€ÇÃFormatÇÃéÊìæ
	template<ViewComponentType VCT,std::uint8_t ComponentSize,std::uint8_t ComponentNum>
	constexpr DXGI_FORMAT GetViewFormat() {
		static_assert(false,"invalid template argument");
	}

#define DefineGetFormat(type,size,num,value)										\
	template<>																		\
	constexpr DXGI_FORMAT GetViewFormat<type,size,value>(){							\
		return value;																\
	}																				\

	DefineGetFormat(ViewComponentType::Float, 32, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	DefineGetFormat(ViewComponentType::Float, 32, 3, DXGI_FORMAT_R32G32B32_FLOAT);
	DefineGetFormat(ViewComponentType::Float, 32, 2, DXGI_FORMAT_R32G32_FLOAT);
	DefineGetFormat(ViewComponentType::Float, 32, 1, DXGI_FORMAT_R32_FLOAT);
	DefineGetFormat(ViewComponentType::Uint, 32, 4, DXGI_FORMAT_R32G32B32A32_UINT);
	DefineGetFormat(ViewComponentType::Uint, 32, 3, DXGI_FORMAT_R32G32B32_UINT);
	DefineGetFormat(ViewComponentType::Uint, 32, 2, DXGI_FORMAT_R32G32_UINT);
	DefineGetFormat(ViewComponentType::Uint, 32, 1, DXGI_FORMAT_R32_UINT);
	DefineGetFormat(ViewComponentType::Float, 16, 4, DXGI_FORMAT_R16G16B16A16_FLOAT);
	DefineGetFormat(ViewComponentType::Float, 16, 2, DXGI_FORMAT_R16G16_FLOAT);
	DefineGetFormat(ViewComponentType::Float, 16, 1, DXGI_FORMAT_R16_FLOAT);
	DefineGetFormat(ViewComponentType::Uint, 16, 4, DXGI_FORMAT_R16G16B16A16_UINT);
	DefineGetFormat(ViewComponentType::Uint, 16, 2, DXGI_FORMAT_R16G16_UINT);
	DefineGetFormat(ViewComponentType::Uint, 16, 1, DXGI_FORMAT_R16_UINT);
	DefineGetFormat(ViewComponentType::Uint, 8, 4, DXGI_FORMAT_R8G8B8A8_UINT);
	DefineGetFormat(ViewComponentType::Uint, 8, 2, DXGI_FORMAT_R8G8_UINT);
	DefineGetFormat(ViewComponentType::Uint, 8, 1, DXGI_FORMAT_R8_UINT);
	DefineGetFormat(ViewComponentType::UnsignedNormalizedFloat, 8, 4, DXGI_FORMAT_R8G8B8A8_UNORM);
	DefineGetFormat(ViewComponentType::UnsignedNormalizedFloat, 8, 2, DXGI_FORMAT_R8G8_UNORM);
	DefineGetFormat(ViewComponentType::UnsignedNormalizedFloat, 8, 1, DXGI_FORMAT_R8_UNORM);

#undef DefineGetFormat

	//DepthStencilÇÃDescriptorHeapÇ…ViewÇçÏÇÈéûÇ…égóp
	template<std::uint8_t ComponentSize, std::uint8_t ComponentNum>
	constexpr DXGI_FORMAT GetDepthStencilViewFormat() {
		static_assert(false, "invalid template argument");
	}

	template<>
	constexpr DXGI_FORMAT GetDepthStencilViewFormat<16,1>() {
		return DXGI_FORMAT_D16_UNORM;
	}

	template<>
	constexpr DXGI_FORMAT GetDepthStencilViewFormat<32, 1>() {
		return DXGI_FORMAT_D32_FLOAT;;
	}


	inline void CreateConstantBufferView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, std::uint32_t sizeInBytes)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
		desc.BufferLocation = resource->GetGPUVirtualAddress();
		desc.SizeInBytes = sizeInBytes;
		device->CreateConstantBufferView(&desc, cpuHandle);
	}

	template<ViewComponentType VCT,typename PrimitiveFormat>
	inline void CreateTexture2DShaderResourceView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSline, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = GetViewFormat<VCT, PrimitiveFormat::componentSize, PrimitiveFormat::componentNum>();
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = mipLevels;
		desc.Texture2D.MostDetailedMip = mostDetailedMip;
		desc.Texture2D.PlaneSlice = planeSline;
		desc.Texture2D.ResourceMinLODClamp = resourceMinLODClamp;
		device->CreateShaderResourceView(resource, &desc, cpuHandle);
	}

	template<ViewComponentType VCT,typename PrimitiveFormat>
	inline void CreateTexture2DArrayShaderResourceView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = GetViewFormat<VCT, PrimitiveFormat::componentSize, PrimitiveFormat::componentNum>();
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

	template<ViewComponentType VCT, typename PrimitiveFormat>
	inline void CreateTextureCubeShaderResourceView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipLevels, std::uint32_t mostDetailedMip, std::uint32_t planeSlice, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = GetViewFormat<VCT, PrimitiveFormat::componentSize, PrimitiveFormat::componentNum>();
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

	template<ViewComponentType VCT,typename PrimitiveFormat>
	inline void CreateTextre2DUnorderedAccessResourceView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		ID3D12Resource* counterResource, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Format = GetViewFormat<VCT, PrimitiveFormat::componentSize, PrimitiveFormat::componentNum>();
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;
		desc.Texture2D.PlaneSlice = planeSlice;
		device->CreateUnorderedAccessView(resource, counterResource, &desc, cpuHandle);
	}

	template<typename PrimitiveFormat>
	inline void CreateTexture2DDepthStencilView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t mipSlice)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Format = GetDepthStencilViewFormat<PrimitiveFormat::componentSize, PrimitiveFormat::componentNum>();
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;
		device->CreateDepthStencilView(resource, &desc, cpuHandle);
	}
	
	template<typename PrimitiveFormat>
	inline void CreateTexture2DArrayDepthStencilView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Format = GetDepthStencilViewFormat<PrimitiveFormat::componentSize, PrimitiveFormat::componentNum>();
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipSlice = mipSlice;
		device->CreateDepthStencilView(resource, &desc, cpuHandle);
	}

	template<ViewComponentType VCT, typename PrimitiveFormat>
	inline void CreateTextre2DRenderTargetView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = GetViewFormat<VCT, PrimitiveFormat::componentSize, PrimitiveFormat::componentNum>();
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;
		desc.Texture2D.PlaneSlice = planeSlice;
		device->CreateRenderTargetView(resource, &desc, cpuHandle);
	}

	template<ViewComponentType VCT, typename PrimitiveFormat>
	inline void CreateTextre2DArrayRenderTargetView(ID3D12Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		std::uint32_t arraySize, std::uint32_t firstArraySlice, std::uint32_t mipSlice, std::uint32_t planeSlice)
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = GetViewFormat<VCT, PrimitiveFormat::componentSize, PrimitiveFormat::componentNum>();
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipSlice = mipSlice;
		desc.Texture2DArray.PlaneSlice = planeSlice;
		device->CreateRenderTargetView(resource, &desc, cpuHandle);
	}

}