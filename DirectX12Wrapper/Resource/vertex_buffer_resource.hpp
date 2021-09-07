#pragma once
#include"buffer_resource.hpp"
#include"mapped_resource.hpp"

namespace DX12
{
	template<typename FormatTuple>
	class vertex_buffer_resource : public buffer_resource<resource_heap_property::Upload>
	{
		D3D12_VERTEX_BUFFER_VIEW buffer_view{};

	public:
		void initialize(device& device, std::uint32_t num);

		const D3D12_VERTEX_BUFFER_VIEW& get_view() const noexcept;

		using mapped_resource_type = formats_mapped_resource<FormatTuple>;
	};



	//
	//
	//

	template<typename FormatTuple>
	void vertex_buffer_resource<FormatTuple>::initialize(device& device, std::uint32_t num)
	{
		constexpr auto strideSum = FormatTuple::get_formats_stride();

		buffer_resource::initialize(device, strideSum * num);

		buffer_view.BufferLocation = get()->GetGPUVirtualAddress();
		buffer_view.SizeInBytes = strideSum * num;
		buffer_view.StrideInBytes = strideSum;
	}

	template<typename FormatTuple>
	inline const D3D12_VERTEX_BUFFER_VIEW& vertex_buffer_resource<FormatTuple>::get_view() const noexcept
	{
		return buffer_view;
	}

}