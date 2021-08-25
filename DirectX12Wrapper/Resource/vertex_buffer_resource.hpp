#pragma once
#include"buffer_resource.hpp"

namespace DX12
{
	class vertex_buffer_resource : public buffer_resource
	{
		D3D12_VERTEX_BUFFER_VIEW buffer_view{};

	public:
		void initialize(Device* device, std::uint32_t size, std::uint32_t stride);

		const D3D12_VERTEX_BUFFER_VIEW& get_view() const noexcept;
	};

	//
	//
	//

	void vertex_buffer_resource::initialize(Device* device, std::uint32_t size, std::uint32_t stride)
	{
		buffer_resource::initialize(device, size);

		buffer_view.BufferLocation = get()->GetGPUVirtualAddress();
		buffer_view.SizeInBytes = size;
		buffer_view.StrideInBytes = stride;
	}

	inline const D3D12_VERTEX_BUFFER_VIEW& DX12::vertex_buffer_resource::get_view() const noexcept
	{
		return buffer_view;
	}
}