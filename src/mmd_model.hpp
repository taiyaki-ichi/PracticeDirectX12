#pragma once
#include"include/pmx_data_struct.hpp"
#include"DirectX12/texture_shader_resource.hpp"
#include"DirectX12/color_texture.hpp"
#include"scene_data.hpp"
#include<memory>
#include<vector>
#include<string>

namespace ichi
{
	class device;
	class vertex_buffer;
	class index_buffer;
	class constant_buffer_resource;
	class command_list;
	class descriptor_heap;
	class white_texture_resource;
	class black_texture_resource;
	class pipeline_state;


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
		std::vector<std::unique_ptr<texture_shader_resource>> m_texture{};

		//シェーダに送る用
		std::unique_ptr<constant_buffer_resource> m_scene_data_resource{};

		//今のところfloat4,float3,float,float3のmaterial
		//マテリアルの描写毎に定数バッファをリセットしビューを作製
		std::vector<std::unique_ptr<constant_buffer_resource>> m_material_resource{};

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

		//ディスクリプタヒープ
		//SetDescriptorHeapsを複数回呼び出すとうまくいかなかったのであきらめて1つにまとめた
		std::unique_ptr<descriptor_heap> m_descriptor_heap{};

		//マテリアルごとのgpuハンドルの先頭
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_matarial_root_gpu_handle{};

		//とりあえず白と黒のテクスチャもメンバとして持たせる
		std::unique_ptr<white_texture_resource> m_white_texture_resource{};
		std::unique_ptr<black_texture_resource> m_black_texture_resource{};
		//グラデーションも
		std::unique_ptr<gray_gradation_texture_resource> m_gray_gradation_texture_resource{};

		//シャドウマップ用の深度バッファ
		//とりあえず、生ぽ
		ID3D12Resource* m_light_depth_resource = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE m_light_depth_resource_gpu_handle{};


	public:
		mmd_model() = default;
		~mmd_model();

		//コマンドリストはテクスチャのコピー用
		bool initialize(device*,const MMDL::pmx_model<std::wstring>&,command_list*);


		//
		//仮
		//drawはコマンドリストのメンバ関数にしたいな
		//
		void draw(command_list* cl);

		void map_scene_data(const scene_data&);

	};

}