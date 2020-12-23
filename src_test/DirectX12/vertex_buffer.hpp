#pragma once
#include"utility.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	class vertex_buffer
	{
		ID3D12Resource* m_resource = nullptr;
		D3D12_VERTEX_BUFFER_VIEW m_buffer_view{};

	public:
		vertex_buffer() = default;
		~vertex_buffer();

		bool initialize(device*, unsigned int size, unsigned int stride);

		//�f�[�^�̃}�b�v
		template<typename Value, size_t N>
		bool map(const Value(&a)[N]){
			return map(m_resource, a[N]);
		}

		const D3D12_VERTEX_BUFFER_VIEW& get_view();
	};


}