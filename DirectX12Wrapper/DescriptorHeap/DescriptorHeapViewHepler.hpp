#pragma once
#include"../Device.hpp"

#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	//前方宣言
	namespace DescriptorHeapTypeTag {
		struct CBV_SRV_UAV;
		struct DSV;
		struct RTV;
	}
	namespace DescriptorHeapViewTag {
		struct ConstantBuffer;
		struct DepthStencilBuffer;
		struct Float4ShaderResource;
		struct FloatShaderResource;
		struct CubeMapResource;
		struct CubeMapDepthStencilBuffer;
		struct UnorderedAccessResource;
	}

	//ビューを作成する関数
	template<typename DescriptorHeapTypeTag, typename DescriptorHeapViewTag>
	inline bool CreateView(Device*, ID3D12Resource*, const D3D12_CPU_DESCRIPTOR_HANDLE&);

	//
	//
	//

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, DescriptorHeapViewTag::ConstantBuffer>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
		cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(resource->GetDesc().Width);

		device->Get()->CreateConstantBufferView(&cbvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool  CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, DescriptorHeapViewTag::Float4ShaderResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = resource->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		device->Get()->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool  CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, DescriptorHeapViewTag::DepthStencilBuffer>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC resDesc{};
		resDesc.Format = DXGI_FORMAT_R32_FLOAT;
		resDesc.Texture2D.MipLevels = 1;
		resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		device->Get()->CreateShaderResourceView(resource, &resDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, DescriptorHeapViewTag::FloatShaderResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		device->Get()->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, DescriptorHeapViewTag::CubeMapResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle) 
	{
		const auto& desc = resource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC resDesc{};
		resDesc.Format = desc.Format;
		resDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		resDesc.Texture2DArray.MipLevels = desc.MipLevels;
		resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

		device->Get()->CreateShaderResourceView(resource, &resDesc, cpuHandle);

		return true;
	}

	//
	template<>
	inline bool CreateView<DescriptorHeapTypeTag::CBV_SRV_UAV, DescriptorHeapViewTag::UnorderedAccessResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		const auto& desc = resource->GetDesc();
		D3D12_UNORDERED_ACCESS_VIEW_DESC resDesc{};
		resDesc.Format = desc.Format;
		//
		resDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		resDesc.Texture2D.MipSlice = 0;
		resDesc.Texture2D.PlaneSlice = 0;

		device->Get()->CreateUnorderedAccessView(resource, nullptr, &resDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool  CreateView<DescriptorHeapTypeTag::DSV, DescriptorHeapViewTag::DepthStencilBuffer>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		device->Get()->CreateDepthStencilView(resource, &dsvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool  CreateView<DescriptorHeapTypeTag::DSV, DescriptorHeapViewTag::CubeMapDepthStencilBuffer>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.ArraySize = 6;
		dsvDesc.Texture2DArray.FirstArraySlice = 0;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		device->Get()->CreateDepthStencilView(resource, &dsvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::RTV, DescriptorHeapViewTag::Float4ShaderResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->Get()->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::RTV, DescriptorHeapViewTag::FloatShaderResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->Get()->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool CreateView<DescriptorHeapTypeTag::RTV, DescriptorHeapViewTag::CubeMapResource>
		(Device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		const auto& desc = resource->GetDesc();
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = desc.Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;

		device->Get()->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

		return true;
	}


}