#pragma once
#include"upload_resource.hpp"

namespace DX12
{
	//インデックスバッファ
	//インデックスはunsigned int 固定で
	class index_buffer : public upload_resource
	{
		D3D12_INDEX_BUFFER_VIEW m_buffer_view{};

	public:
		bool initialize(device*, unsigned int size);

		const D3D12_INDEX_BUFFER_VIEW& get_view() const noexcept;
	};

	
}