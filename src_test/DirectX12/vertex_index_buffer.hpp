#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//とりあえず
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

		//初期化
		//device内で呼び出される
		bool initialize(device* device, unsigned int size) {

		}

		//データのマップ
		template<typename T>
		bool map(T&&) {

		}

		const ViewType& get_view() const {
			return m_vertex_buffer_view;
		}
	};

}