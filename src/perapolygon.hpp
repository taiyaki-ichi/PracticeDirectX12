#pragma once
#include"DirectX12/resource.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include<utility>
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include"DirectX12/color_texture.hpp"

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
	class resource;

	class perapolygon
	{
		ID3D12RootSignature* m_root_signature = nullptr;
		ID3D12PipelineState* m_pipeline_state = nullptr;
		ID3D12PipelineState* m_blur_pipeline_state = nullptr;
	
		//リソースたち
		constexpr static unsigned int RESOURCE_NUM = 5;
		resource m_resource[RESOURCE_NUM]{};
		enum ResourceIndex {
			COLOR,//色
			NORMAL,//法線
			BLOOM,//ブルーム用
			SHRINK_BLOOM,//ブルーム用の縮小されたリソース
			DOF//被写界深度ぼかしフィルタ用
		};

		//ぺらポリゴンのリソースにデータを書き込む用
		descriptor_heap<descriptor_heap_type::RTV> m_rtv_descriptor_heap{};

		//ぺらポリゴンのリソースをテクスチャとして解釈させるための
		descriptor_heap<descriptor_heap_type::CBV_SRV_UAV> m_cbv_srv_usv_descriptor_heap{};

		//ぺらポリゴンをバックバッファに描写する際セットする用
		vertex_buffer m_vertex_buffer{};

		//レンダーターゲットに指定する時に使う
		static constexpr unsigned int RENDER_TARGET_HANDLE_NUM = 3;
		D3D12_CPU_DESCRIPTOR_HANDLE m_render_target_view_cpu_handle_array[RENDER_TARGET_HANDLE_NUM]{};

	public:
		perapolygon() = default;
		~perapolygon();

		//深度バッファのViewを作りたいので引数に深度バッファ
		bool initialize(device*, ID3D12Resource* depthResource);

		//レンダーターゲットの数とその先頭のハンドルの取得
		//ぺらポリゴンをレンダーターゲットとする際に使用することを想定
		std::pair<int, D3D12_CPU_DESCRIPTOR_HANDLE*> get_render_target_info();

		//すべてのリソースのリソースバリア
		void all_resource_barrior(command_list*, D3D12_RESOURCE_STATES);

		//リソースたちの初期化
		void clear(command_list*);

		//ぺらポリゴンを書き込む
		void draw(command_list*);

		//ぼかしを行うため縮小バッファへの書き込み
		void draw_shrink_texture_for_blur(command_list*);
	};

}