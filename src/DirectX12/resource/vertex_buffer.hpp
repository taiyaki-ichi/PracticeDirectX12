#pragma once
#include"upload_resource.hpp"

namespace DX12
{
	//頂点バッファ
	class vertex_buffer : public upload_resource
	{
		D3D12_VERTEX_BUFFER_VIEW m_buffer_view{};

	public:
		bool initialize(device*, unsigned int size, unsigned int stride);

		const D3D12_VERTEX_BUFFER_VIEW& get_view() const noexcept;
	};
}