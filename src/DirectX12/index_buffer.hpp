#pragma once
#include"resource.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class index_buffer : public resource
	{
		D3D12_INDEX_BUFFER_VIEW m_buffer_view{};

	public:
		bool initialize(device*, unsigned int size);

		const D3D12_INDEX_BUFFER_VIEW& get_view() const noexcept;
	};
}