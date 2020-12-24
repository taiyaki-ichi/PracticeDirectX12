#pragma once
#include"utility.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	class index_buffer
	{
		ID3D12Resource* m_resource = nullptr;
		D3D12_INDEX_BUFFER_VIEW m_buffer_view{};

	public:
		index_buffer() = default;
		~index_buffer();

		bool initialize(device*,unsigned int size);

		//データのマップ
		template<typename T>
		bool map(T&& t) {
			return map_func(m_resource, std::forward<T>(t));
		}

		const D3D12_INDEX_BUFFER_VIEW& get_view() const noexcept;
	};
}