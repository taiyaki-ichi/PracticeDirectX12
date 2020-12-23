#pragma once
#include"device.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//とりあえずテンプレートでそれぞれ実装しておく
	template<typename ViewType>
	class vertex_index_buffer_base
	{
		ID3D12Resource* m_resource = nullptr;
		ViewType m_buffer_view{};

	public:
		vertex_index_buffer_base() = default;
		~vertex_index_buffer_base() {
			if (m_resource)
				m_resource->Release();
		}

		//初期化
		//device内で呼び出される
		bool initialize(device* d, unsigned int size) {

			D3D12_HEAP_PROPERTIES heapprop{};
			heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

			D3D12_RESOURCE_DESC resdesc{};
			resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resdesc.Width = size;
			resdesc.Height = 1;
			resdesc.DepthOrArraySize = 1;
			resdesc.MipLevels = 1;
			resdesc.Format = DXGI_FORMAT_UNKNOWN;
			resdesc.SampleDesc.Count = 1;
			resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			
			//失敗
			if (FAILED(d->get()->CreateCommittedResource(
				&heapprop,
				D3D12_HEAP_FLAG_NONE,
				&resdesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_resource)))) {

				std::cout << "vert_index_buufer init is failed\n";
				return false;
			}
			
			if constexpr (std::is_same_v<ViewType, D3D12_VERTEX_BUFFER_VIEW>) {
				m_buffer_view
			}

		}

		//データのマップ
		template<typename Value, size_t N>
		bool map(const Value(&a)[N])
		{
			Value* target = nullptr;
			auto result = m_resource->Map(0, nullptr, (void**)&target);
			//失敗したとき
			if (FAILED(result)) {
				std::cout <<"vi_buufer map is failed\n";
				return false;
			}

			std::copy(std::begin(a), std::end(a), target);
			buffer->Unmap(0, nullptr);

			return true;
		}

		/*
		template<typename T>
		bool map(T&&) {

		}
		*/

		const ViewType& get_view() const {
			return m_buffer_view;
		}
	};


	using vertex_buffer = vertex_index_buffer_base<D3D12_VERTEX_BUFFER_VIEW>;
	using index_buffer = vertex_index_buffer_base<D3D12_INDEX_BUFFER_VIEW>;

}