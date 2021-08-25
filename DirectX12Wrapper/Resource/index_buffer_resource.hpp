#pragma once
#include"buffer_resource.hpp"
#include"../Format.hpp"

namespace DX12
{
	class index_buffer_resource : public buffer_resource
	{
		D3D12_INDEX_BUFFER_VIEW index_buffer_view{};

	public:
		void initialize(Device* device, std::uint32_t size, format f);

		const D3D12_INDEX_BUFFER_VIEW& get_view() const noexcept;
	};

	//
	//
	//

	void index_buffer_resource::initialize(Device* device, std::uint32_t size, format f)
	{
		buffer_resource::initialize(device, size);

		index_buffer_view.BufferLocation = get()->GetGPUVirtualAddress();
		index_buffer_view.Format = get_dxgi_format(f.type, f.size, f.num).value();
		index_buffer_view.SizeInBytes = size;
	}

	inline const D3D12_INDEX_BUFFER_VIEW& index_buffer_resource::get_view() const noexcept
	{
		return index_buffer_view;
	}
}