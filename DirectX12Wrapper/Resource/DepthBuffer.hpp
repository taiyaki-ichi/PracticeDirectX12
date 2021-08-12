#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	class DepthBuffer : public ResourceBase
	{
	public:
		void Initialize(Device*, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize = 1);
	};

	template<>
	struct DefaultViewTypeTraits<DepthBuffer> {
		using Type = DescriptorHeapViewTag::DepthStencilBuffer;
	};

	//
	//
	//

	inline void DepthBuffer::Initialize(Device* device, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize)
	{
		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthResDesc.Width = width;
		depthResDesc.Height = height;
		depthResDesc.DepthOrArraySize = depthOrArraySize;
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

		ResourceBase::Initialize(device,
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			ResourceState::DepthWrite,
			&depthClearValue
		);
	}
}