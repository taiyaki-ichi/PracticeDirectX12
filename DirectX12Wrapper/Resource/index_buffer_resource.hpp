#pragma once
#include"buffer_resource.hpp"
#include"../format.hpp"

namespace DX12
{
	template<typename Format>
	class index_buffer_resource : public buffer_resource
	{
		D3D12_INDEX_BUFFER_VIEW index_buffer_view{};

	public:
		void initialize(device& device, std::uint32_t num);

		const D3D12_INDEX_BUFFER_VIEW& get_view() const noexcept;

		static_assert(get_dxgi_format(Format::component_type, Format::component_size, Format::component_num));
	};

	//
	//
	//

	template<typename Format>
	void index_buffer_resource<Format>::initialize(device& device, std::uint32_t num)
	{
		buffer_resource::initialize(device, Format::component_size / 8 * num);

		index_buffer_view.BufferLocation = get()->GetGPUVirtualAddress();
		index_buffer_view.Format = get_dxgi_format(Format::component_type, Format::component_size, Format::component_num).value();
		index_buffer_view.SizeInBytes = Format::component_size / 8 * num;
	}

	template<typename Format>
	inline const D3D12_INDEX_BUFFER_VIEW& index_buffer_resource<Format>::get_view() const noexcept
	{
		return index_buffer_view;
	}
}