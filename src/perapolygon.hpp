#pragma once
#include<utility>
#include<memory>
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace ichi
{
	class device;
	template<typename>
	class descriptor_heap;
	namespace descriptor_heap_type {
		struct RTV;
		struct CBV_SRV_UAV;
	}
	class vertex_buffer;
	class command_list;


	class perapolygon
	{
		ID3D12RootSignature* m_root_signature = nullptr;
		ID3D12PipelineState* m_pipeline_state = nullptr;
		ID3D12PipelineState* m_blur_pipeline_state = nullptr;

		//実際のリソース、色用
		ID3D12Resource* m_color_resource = nullptr;
		//法線用
		ID3D12Resource* m_normal_resource = nullptr;
		//ブルーム用
		ID3D12Resource* m_bloom_resource = nullptr;
		//ブルームの際使用する縮小されたバッファ
		ID3D12Resource* m_shrink_bloom_resource = nullptr;


		//ぺらポリゴンのリソースにデータを書き込む用
		std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>> m_rtv_descriptor_heap{};

		//ぺらポリゴンのリソースをテクスチャとして解釈させるための
		std::unique_ptr<descriptor_heap<descriptor_heap_type::CBV_SRV_UAV>> m_cbv_srv_usv_descriptor_heap{};

		//ぺらポリゴンをバックバッファに描写する際セットする用
		std::unique_ptr<vertex_buffer> m_vertex_buffer{};

		//レンダーターゲットに指定する時に使う
		D3D12_CPU_DESCRIPTOR_HANDLE m_render_target_view_cpu_handle_array[3]{};

	public:
		perapolygon() = default;
		~perapolygon();

		bool initialize(device*);

		//レンダーターゲットの数とその先頭のハンドルの取得
		//ぺらポリゴンをレンダーターゲットとする際に使用することを想定
		std::pair<int, D3D12_CPU_DESCRIPTOR_HANDLE*> get_render_target_info();

		//ぺらポリゴンへ書き込む際のリソースバリア
		void begin_drawing_resource_barrier(command_list*);
		//ぺらポリゴンへの書き込みが終わった時に呼び出すリソースバリア
		void end_drawing_resource_barrier(command_list*);

		//リソースたちの初期化
		void clear(command_list*);

		//ぺらポリゴンを書き込む
		void draw(command_list*);

		//ぼかしを行うため縮小バッファへの書き込み
		void draw_shrink_texture_for_blur(command_list*);
	};

}