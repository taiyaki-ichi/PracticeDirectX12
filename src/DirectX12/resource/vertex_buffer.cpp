#include"vertex_buffer.hpp"

namespace DX12
{
	bool vertex_buffer::initialize(device* device, unsigned int size, unsigned int stride)
	{
		
		if (!upload_resource::initialize(device, size))
			return false;

		m_buffer_view.BufferLocation = get()->GetGPUVirtualAddress();//バッファの仮想アドレス
		m_buffer_view.SizeInBytes = size;//全バイト数
		m_buffer_view.StrideInBytes = stride;//1頂点あたりのバイト数

		return true;
	}
	const D3D12_VERTEX_BUFFER_VIEW& vertex_buffer::get_view() const noexcept
	{
		return m_buffer_view;
	}

}