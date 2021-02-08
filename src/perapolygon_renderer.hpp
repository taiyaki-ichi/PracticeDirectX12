#pragma once
#include"DirectX12/resource_type_tag.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;
	class depth_buffer;

	//ぺらポリゴンのレンダラー関係
	class perapolygon_renderer
	{
		//書き込み用のリソースのビューを作る
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		//実際のリソース
		ID3D12Resource* m_resource = nullptr;

	public:
		perapolygon_renderer() = default;
		~perapolygon_renderer();

		bool initialize(device*);

		//リソースへの描写の開始と終わり
		void begin_drawing(command_list* cl, depth_buffer* db);
		void end_drawing(command_list* cl);

		//リソースのクリア
		//と思ったけど、initでクリアカラーのしたからいらないかも
		//とりあえずナシ
		//と思ったけど必要だったっぽい
		void clear(command_list* cl);

		//リソースに描写したのちディスクリプターヒープにビューを作る用
		//として、想定
		//分けた方がよさそうだが
		using create_view_type = typename create_view_type::SRV;
		ID3D12Resource* get() noexcept;
	};

}