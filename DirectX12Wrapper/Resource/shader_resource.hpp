#pragma once
#include"resource.hpp"

namespace DX12
{
	template<typename Format,resource_flag... Flags>
	class shader_resource : public resource<resource_dimention::Texture2D, Format, resource_heap_property::Default, Flags...>
	{
	public:
		void initialize(Device* device, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize, std::uint16_t mipLevels, const D3D12_CLEAR_VALUE* clearValue = nullptr);
	};

	//
	//
	//

	template<typename Format, resource_flag ...Flags>
	inline void shader_resource<Format, Flags...>::initialize(Device* device, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize, std::uint16_t mipLevels, const D3D12_CLEAR_VALUE* clearValue)
	{
		resource<resource_dimention::Texture2D, Format, resource_heap_property::Default, Flags...>::initialize(device,
			width, height, depthOrArraySize, mipLevels, clearValue);
	}
}