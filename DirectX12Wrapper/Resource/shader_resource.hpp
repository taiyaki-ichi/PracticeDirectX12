#pragma once
#include"resource.hpp"

namespace DX12
{
	template<typename TypelessFormat,resource_flag... Flags>
	class shader_resource : public resource<resource_dimention::Texture2D, TypelessFormat, resource_heap_property::Default, Flags...>
	{
	public:
		void initialize(Device* device, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize, std::uint16_t mipLevels, const D3D12_CLEAR_VALUE* clearValue = nullptr);
	};

	//
	//
	//

	template<typename TypelessFormat, resource_flag ...Flags>
	inline void shader_resource<TypelessFormat, Flags...>::initialize(Device* device, std::uint32_t width, std::uint32_t height, std::uint16_t depthOrArraySize, std::uint16_t mipLevels, const D3D12_CLEAR_VALUE* clearValue)
	{
		resource<resource_dimention::Texture2D, TypelessFormat, resource_heap_property::Default, Flags...>::initialize(device,
			width, height, depthOrArraySize, mipLevels, clearValue);
	}
}