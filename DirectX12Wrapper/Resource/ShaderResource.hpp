#pragma once
#include"ResourceBase.hpp"
#include"DescriptorHeap/DescripotrHeapViewTag.hpp"
#include"../Format.hpp"
#include<array>
#include<variant>

namespace DX12
{
	template<Format F>
	struct ClearValueTraits {
		using Type;
	};

	template<>
	struct ClearValueTraits<Format::R8G8B8A8> {
		using Type = std::array<float, 4>;
	};

	template<>
	struct ClearValueTraits<Format::R32_FLOAT> {
		using Type = float;
	};


	template<Format F,std::size_t DepthOrArraySize>
	class ShaderResourceBase : public ResourceBase
	{
	public:
		void Initialize(Device*, std::size_t width, std::size_t height, std::optional<typename ClearValueTraits<F>::Type> = std::nullopt);
	};



	using Float4ShaderResource = ShaderResourceBase<Format::R8G8B8A8, 1>;
	using FloatShaderResource = ShaderResourceBase<Format::R32_FLOAT, 1>;
	using CubeMapShaderResource = ShaderResourceBase<Format::R8G8B8A8, 6>;

	template<>
	struct DefaultViewTypeTraits<Float4ShaderResource> {
		using Type = DescriptorHeapViewTag::Float4ShaderResource;
	};

	template<>
	struct DefaultViewTypeTraits<FloatShaderResource> {
		using Type = DescriptorHeapViewTag::FloatShaderResource;
	};

	template<>
	struct DefaultViewTypeTraits<CubeMapShaderResource> {
		using Type = DescriptorHeapViewTag::CubeMapResource;
	};

	//
	//
	//

	template<Format F, std::size_t DepthOrArraySize>
	inline void ShaderResourceBase<F, DepthOrArraySize>::Initialize(Device* device, std::size_t width, std::size_t height, std::optional<typename ClearValueTraits<F>::Type> cv)
	{
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resdesc.Width = width;
		resdesc.Height = height;
		resdesc.Format = static_cast<DXGI_FORMAT>(F);
		resdesc.DepthOrArraySize = DepthOrArraySize;
		resdesc.SampleDesc.Count = 1;
		//
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		resdesc.MipLevels = 1;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resdesc.Alignment = 0;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = static_cast<DXGI_FORMAT>(F);

		//‚Æ‚è‚ ‚¦‚¸
		if (cv)
		{
			if constexpr (F == Format::R8G8B8A8) 
				std::copy(std::begin(cv.value()), std::end(cv.value()), std::begin(clearValue.Color));
			if constexpr (F == Format::R32_FLOAT)
				clearValue.DepthStencil.Depth = cv.value();
		}

		return ResourceBase::Initialize(device,
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			ResourceState::PixcelShaderResource, 
			&clearValue
		);
	}

}