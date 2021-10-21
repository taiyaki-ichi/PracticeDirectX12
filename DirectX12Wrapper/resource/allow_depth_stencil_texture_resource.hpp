#pragma once
#include"resource_state.hpp"
#include"../device.hpp"
#include"../format.hpp"
#include<optional>
#include<variant>
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace DX12
{
	template<std::uint32_t Dimention, typename Format>
	class allow_depth_stencil_texture_resource
	{
		release_unique_ptr<ID3D12Resource> resource_ptr{};
		resource_state state{};

		float depth_clear_value = 0.f;
		std::uint8_t stencil_clear_value = 0;

	public:
		void initialize(device&, std::uint32_t width, std::uint32_t height,
			std::uint16_t depthOrArraySize, std::uint16_t mipLevels, float depthClearValue, std::uint8_t stencilClearValue, bool denyShaderResource = false);

		ID3D12Resource* get() noexcept;

		resource_state get_state() const noexcept;
		void set_state(resource_state) noexcept;

		std::pair<float, std::uint8_t> get_clear_value() const noexcept;

		using format = Format;
	};


	template<typename Format>
	using allow_depth_stencil_texture_1D_resource = allow_depth_stencil_texture_resource<1, Format>;

	template<typename Format>
	using allow_depth_stencil_texture_2D_resource = allow_depth_stencil_texture_resource<2, Format>;

	template<typename Format>
	using allow_depth_stencil_texture_3D_resource = allow_depth_stencil_texture_resource<3, Format>;

	//
	//
	//

	template<std::uint32_t Dimention, typename Format>
	inline void allow_depth_stencil_texture_resource<Dimention, Format>::initialize(device& d, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize, std::uint16_t mipLevels, float depthClearValue, std::uint8_t stencilClearValue, bool denyShaderResource)
	{
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = width;
		resourceDesc.Height = height;
		resourceDesc.DepthOrArraySize = depthOrArraySize;
		resourceDesc.MipLevels = mipLevels;
		resourceDesc.Format = get_resource_format<Format>().value();
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		switch (Dimention)
		{
		case 1:
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			break;
		case 2:
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		case 3:
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			break;
		default:
			break;
		}

		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if (denyShaderResource)
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = get_depth_stencil_dxgi_format(Format::component_type, Format::component_size, Format::component_num).value();
		clearValue.DepthStencil.Depth = depthClearValue;
		clearValue.DepthStencil.Stencil = stencilClearValue;

		state = resource_state::Common;
		depth_clear_value = depthClearValue;
		stencil_clear_value = stencilClearValue;

		ID3D12Resource* tmp = nullptr;
		if (FAILED(d.get()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			static_cast<D3D12_RESOURCE_STATES>(state),
			&clearValue,
			IID_PPV_ARGS(&tmp))))
		{
			THROW_EXCEPTION("");
		}
		resource_ptr.reset(tmp);
	}

	template<std::uint32_t Dimention, typename Format>
	inline ID3D12Resource* allow_depth_stencil_texture_resource<Dimention, Format>::get() noexcept
	{
		return resource_ptr.get();
	}

	template<std::uint32_t Dimention, typename Format>
	inline resource_state allow_depth_stencil_texture_resource<Dimention, Format>::get_state() const noexcept
	{
		return state;
	}

	template<std::uint32_t Dimention, typename Format>
	inline void allow_depth_stencil_texture_resource<Dimention, Format>::set_state(resource_state s) noexcept
	{
		state = s;
	}

	template<std::uint32_t Dimention, typename Format>
	inline std::pair<float, std::uint8_t> allow_depth_stencil_texture_resource<Dimention, Format>::get_clear_value() const noexcept
	{
		return { depth_clear_value,stencil_clear_value };
	}
}