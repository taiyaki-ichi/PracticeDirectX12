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
	template<std::uint32_t Dimention,typename Format>
	class texture_resource
	{
		release_unique_ptr<ID3D12Resource> resource_ptr{};
		resource_state state{};

	public:
		void initialize(device&, std::uint32_t width, std::uint32_t height,
			std::uint16_t depthOrArraySize, std::uint16_t mipLevels, bool allowUnorderAccess = false, bool denyShaderResource = false);

		ID3D12Resource* get() noexcept;

		resource_state get_state() const noexcept;
		void set_state(resource_state) noexcept;

		using format = Format;
	};


	template<typename Format>
	using texture_1D_resource = texture_resource<1, Format>;

	template<typename Format>
	using texture_2D_resource = texture_resource<2, Format>;

	template<typename Format>
	using texture_3D_resource = texture_resource<3, Format>;

	//
	//
	//

	template<std::uint32_t Dimention, typename Format>
	inline void texture_resource<Dimention, Format>::initialize(device& d, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize, std::uint16_t mipLevels, bool allowUnorderAccess, bool denyShaderResource)
	{
		//texture_resourceのヒーププロパティはDefalt固定
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

		if (allowUnorderAccess)
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		if (denyShaderResource)
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		state = resource_state::Common;

		ID3D12Resource* tmp = nullptr;
		if (FAILED(d.get()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			static_cast<D3D12_RESOURCE_STATES>(state),
			nullptr,
			IID_PPV_ARGS(&tmp))))
		{
			THROW_EXCEPTION("");
		}
		resource_ptr.reset(tmp);
	}

	template<std::uint32_t Dimention, typename Format>
	inline ID3D12Resource* texture_resource<Dimention, Format>::get() noexcept
	{
		return resource_ptr.get();
	}

	template<std::uint32_t Dimention, typename Format>
	inline resource_state texture_resource<Dimention, Format>::get_state() const noexcept
	{
		return state;
	}

	template<std::uint32_t Dimention, typename Format>
	inline void texture_resource<Dimention, Format>::set_state(resource_state s) noexcept
	{
		state = s;
	}

}