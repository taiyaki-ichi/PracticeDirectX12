#pragma once
#include"ResourceBase.hpp"
#include"DescriptorHeap/DescripotrHeapViewTag.hpp"

namespace DX12
{
	template<DXGI_FORMAT Format,std::size_t DepthOrArraySize>
	class ShaderResourceBase : public ResourceBase
	{
	public:
		void Initialize(Device*, std::size_t width, std::size_t height, std::optional<D3D12_CLEAR_VALUE> = std::nullopt);
	};

	using Float4ShaderResource = ShaderResourceBase<DXGI_FORMAT_R8G8B8A8_UNORM, 1>;
	using FloatShaderResource = ShaderResourceBase<DXGI_FORMAT_R32_FLOAT, 1>;
	using CubeMapShaderResource = ShaderResourceBase<DXGI_FORMAT_R8G8B8A8_UNORM, 6>;

	template<>
	struct ViewTypeTraits<Float4ShaderResource> {
		using Type = DescriptorHeapViewTag::Float4ShaderResource;
	};

	template<>
	struct ViewTypeTraits<FloatShaderResource> {
		using Type = DescriptorHeapViewTag::FloatShaderResource;
	};

	template<>
	struct ViewTypeTraits<CubeMapShaderResource> {
		using Type = DescriptorHeapViewTag::CubeMapResource;
	};

	//
	//
	//

	template<DXGI_FORMAT Format, std::size_t DepthOrArraySize>
	inline void ShaderResourceBase<Format, DepthOrArraySize>::Initialize(Device* device, std::size_t width, std::size_t height, std::optional<D3D12_CLEAR_VALUE> clearValue)
	{
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resdesc.Width = width;
		resdesc.Height = height;
		resdesc.Format = Format;
		resdesc.DepthOrArraySize = DepthOrArraySize;
		resdesc.SampleDesc.Count = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		resdesc.MipLevels = 1;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resdesc.Alignment = 0;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	
		D3D12_CLEAR_VALUE* clear = nullptr;
		if (clearValue)
			clear = &clearValue.value();

		return ResourceBase::Initialize(device,
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			ResourceState::PixcelShaderResource, 
			clear
		);
	}

}