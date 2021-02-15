#include"vertex_buffer.hpp"

namespace DX12
{
	bool vertex_buffer::initialize(device* device, unsigned int size, unsigned int stride)
	{
		
		if (!upload_resource::initialize(device, size))
			return false;

		m_buffer_view.BufferLocation = get()->GetGPUVirtualAddress();//�o�b�t�@�̉��z�A�h���X
		m_buffer_view.SizeInBytes = size;//�S�o�C�g��
		m_buffer_view.StrideInBytes = stride;//1���_������̃o�C�g��

		return true;
	}
	const D3D12_VERTEX_BUFFER_VIEW& vertex_buffer::get_view() const noexcept
	{
		return m_buffer_view;
	}

}