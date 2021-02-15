#include"index_buffer.hpp"

namespace DX12
{

	bool index_buffer::initialize(device* device, unsigned int size)
	{
		if (!upload_resource::initialize(device, size))
			return false;

		m_buffer_view.BufferLocation = get()->GetGPUVirtualAddress();
		m_buffer_view.Format = DXGI_FORMAT_R16_UINT;
		m_buffer_view.SizeInBytes = size;

		return true;
	}

	const D3D12_INDEX_BUFFER_VIEW& index_buffer::get_view() const noexcept {
		return m_buffer_view;
	}
}