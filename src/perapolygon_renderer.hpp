#pragma once
#include"DirectX12/resource_type_tag.hpp"
#include<memory>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;
	template<typename>
	class descriptor_heap;
	namespace descriptor_heap_type {
		struct RTV;
	}

	//ぺらポリゴンのレンダラー関係
	class perapolygon_renderer
	{
		//書き込み用のリソースのビューを作る
		//ID3D12DescriptorHeap* m_descriptor_heap = nullptr;
		std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>> m_descriptor_heap{};

		//実際のリソース
		ID3D12Resource* m_resource = nullptr;

	public:
		perapolygon_renderer() = default;
		~perapolygon_renderer();

		bool initialize(device*);

		//ぺらポリゴンのリソースへの描写の開始と終わり
		void begin_drawing(command_list*, const D3D12_CPU_DESCRIPTOR_HANDLE&);
		void end_drawing(command_list* cl);

		//リソースのクリア
		//と思ったけど、initでクリアカラーのしたからいらないかも
		//とりあえずナシ
		//と思ったけど必要だったっぽい
		void clear(command_list* cl);

		//リソースの取得
		//ぺらポリゴンに描写されたデータを加工するときとかに使う
		ID3D12Resource* ger_resource_ptr() noexcept;
	};

}