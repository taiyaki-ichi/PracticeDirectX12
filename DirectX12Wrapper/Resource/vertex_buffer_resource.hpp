#pragma once
#include"buffer_resource.hpp"

namespace DX12
{
	template<typename... Formats>
	class vertex_buffer_resource : public buffer_resource
	{
		D3D12_VERTEX_BUFFER_VIEW buffer_view{};

	public:
		void initialize(Device& device, std::uint32_t num);

		const D3D12_VERTEX_BUFFER_VIEW& get_view() const noexcept;
	};

	//ƒwƒ‹ƒp
	//utility‚É‚¢‚Ç‚¤‚©‚È
	template<typename Format>
	inline constexpr std::uint32_t get_format_stride();

	//
	//
	//

	template<typename... Formats>
	void vertex_buffer_resource<Formats...>::initialize(Device& device, std::uint32_t num)
	{
		constexpr auto strideSum = (get_format_stride<Formats>() + ...);

		buffer_resource::initialize(device, strideSum * num);

		buffer_view.BufferLocation = get()->GetGPUVirtualAddress();
		buffer_view.SizeInBytes = strideSum * num;
		buffer_view.StrideInBytes = strideSum;
	}

	template<typename... Formats>
	inline const D3D12_VERTEX_BUFFER_VIEW& vertex_buffer_resource<Formats...>::get_view() const noexcept
	{
		return buffer_view;
	}

	template<typename Format>
	constexpr std::uint32_t get_format_stride()
	{
		return Format::component_size / 8 * Format::component_num;
	}
}