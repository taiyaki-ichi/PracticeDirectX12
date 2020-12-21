#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//�Ƃ肠����
	template<typename ViewType>
	class vertex_index_buffer_base
	{
		ID3D12Resource* m_resource = nullptr;
		ViewType m_vertex_buffer_view{};

	public:
		vertex_index_buffer_base() = default;
		~vertex_index_buffer_base() {
			if (m_resource)
				m_resource->Release();
		}

		//������
		//device���ŌĂяo�����
		bool initialize(device* device, unsigned int size) {

		}

		//�f�[�^�̃}�b�v
		template<typename T>
		bool map(T&&) {

		}

		const ViewType& get_view() const {
			return m_vertex_buffer_view;
		}
	};

}