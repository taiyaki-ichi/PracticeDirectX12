#pragma once
#include"include/pmx_data_struct.hpp"
#include"DirectX12/color_texture.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"scene_data.hpp"
#include"DirectX12/resource.hpp"
#include"DirectX12/color_texture.hpp"
#include<memory>
#include<vector>
#include<string>

namespace ichi
{
	class device;
	class vertex_buffer;
	class index_buffer;
	class command_list;
	class resource;

	class mmd_model
	{
		//通常のパイプラインステート
		ID3D12PipelineState* m_pipeline_state = nullptr;
		//シャドウ用のパイプラインステート
		ID3D12PipelineState* m_shadow_pipeline_state = nullptr;
		//ルートシグネチャ
		ID3D12RootSignature* m_root_signature = nullptr;

		//頂点
		std::unique_ptr<vertex_buffer> m_vertex_buffer{};
		//インデックス
		std::unique_ptr<index_buffer> m_index_buffer{};

		//テクスチャ
		//マテリアルによって添え字で指定される
		std::vector<std::unique_ptr<resource>> m_texture{};

		//シーンのデータの定数バッファ
		std::unique_ptr<resource> m_scene_constant_resource{};

		//今のところfloat4,float3,float,float3のmaterial
		//マテリアルの描写毎に定数バッファをリセットしビューを作製
		std::vector<std::unique_ptr<resource>> m_material_constant_resource{};

		//マテリアルの情報保持用
		struct material_info {
			//m_textureのサイズより多いな値は無効
			//この2つはいらないかも
			unsigned int m_texture_index = 0;
			unsigned int m_toon_index = 0;
			//頂点を指すインデックスの数
			int m_vertex_num = 0;
		};
		//マテリアルの情報の保持
		std::vector<material_info> m_material_info{};

		//mmdのすべてのインデックスの数
		size_t m_all_index_num{};

		//ディスクリプタヒープ
		//SetDescriptorHeapsを複数回呼び出すとうまくいかなかったのであきらめて1つにまとめた
		std::unique_ptr<descriptor_heap<descriptor_heap_type::CBV_SRV_UAV>> m_descriptor_heap{};

		//マテリアルごとのgpuハンドルの先頭
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_matarial_root_gpu_handle{};

		//とりあえず白と黒のテクスチャもメンバとして持たせる
		std::unique_ptr<white_texture_resource> m_white_texture_resource{};
		std::unique_ptr<black_texture_resource> m_black_texture_resource{};
		//グラデーションも
		std::unique_ptr<gray_gradation_texture_resource> m_gray_gradation_texture_resource{};

		//深度書き込み用のバッファ
		ID3D12Resource* m_depth_resource = nullptr;
		//シャドウマップ用の深度バッファ
		ID3D12Resource* m_light_depth_resource = nullptr;
		//深度バッファとライト深度バッファ用のディスクリプタヒープ
		//普通のを0番目、ライト用を1番目となるようにViewを作製す
		std::unique_ptr<descriptor_heap<descriptor_heap_type::DSV>> m_depth_descriptor_heap{};

		//描写用のGPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE m_light_depth_gpu_handle{};
	

	public:
		mmd_model() = default;
		~mmd_model();

		//コマンドリストはテクスチャのコピー用
		bool initialize(device*,const MMDL::pmx_model<std::wstring>&,command_list*);

		void draw(command_list* cl);

		void map_scene_data(const scene_data&);

		//ライト深度バッファへの描写
		void draw_light_depth(command_list* cl);


		//これらは仮
		//多分深度バッファは他を描写するとき使いまわしそうだから外に出したい
		D3D12_CPU_DESCRIPTOR_HANDLE get_depth_resource_cpu_handle() const noexcept;
		ID3D12Resource* get_depth_resource() noexcept;

	};

}