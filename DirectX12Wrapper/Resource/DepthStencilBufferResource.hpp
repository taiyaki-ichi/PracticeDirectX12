#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	template<std::uint32_t DepthOrArraySize>
	class DepthStencilBufferResourceBase : public ResourceBase
	{
	public:
		void Initialize(Device*, std::uint32_t width, std::uint32_t height);
	};


	using DepthStencilBufferResource = DepthStencilBufferResourceBase<1>;

	template<>
	struct DefaultViewTypeTraits<DepthStencilBufferResource>{
		using Type = DescriptorHeapViewTag::DepthStencilBuffer;
	};


	using CubeMapDepthStencilBufferResource = DepthStencilBufferResourceBase<6>;

	template<>
	struct DefaultViewTypeTraits<CubeMapDepthStencilBufferResource> {
		using Type = DescriptorHeapViewTag::CubeMapDepthStencilBuffer;
	};

	//
	//
	//

	template<std::uint32_t DepthOrArraySize>
	inline void DepthStencilBufferResourceBase<DepthOrArraySize>::Initialize(Device* device, std::uint32_t width, std::uint32_t height)
	{
		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthResDesc.Width = width;
		depthResDesc.Height = height;
		depthResDesc.DepthOrArraySize = DepthOrArraySize;
		depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		depthResDesc.SampleDesc.Count = 1;
		depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		depthResDesc.MipLevels = 1;
		depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthResDesc.Alignment = 0;

		D3D12_HEAP_PROPERTIES depthHeapProp{};
		depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.f;
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthClearValue.Color[0] = 1.f;
		depthClearValue.Color[1] = 1.f;
		depthClearValue.Color[2] = 1.f;
		depthClearValue.Color[3] = 1.f;

		return ResourceBase::Initialize(device,
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			ResourceState::DepthWrite,
			&depthClearValue
		);
	}

}