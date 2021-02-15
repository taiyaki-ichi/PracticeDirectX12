#pragma once
#include"upload_resource.hpp"

namespace DX12
{
	//�C���f�b�N�X�o�b�t�@
	//�C���f�b�N�X��unsigned int �Œ��
	class index_buffer : public upload_resource
	{
		D3D12_INDEX_BUFFER_VIEW m_buffer_view{};

	public:
		bool initialize(device*, unsigned int size);

		const D3D12_INDEX_BUFFER_VIEW& get_view() const noexcept;
	};

	
}