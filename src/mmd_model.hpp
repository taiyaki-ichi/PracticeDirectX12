#pragma once
#include"include/pmx_data_struct.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"scene_data.hpp"
#include"DirectX12/resource/vertex_buffer.hpp"
#include"DirectX12/resource/index_buffer.hpp"
#include"DirectX12/resource/color_texture.hpp"
#include"DirectX12/resource/texture_resource.hpp"
#include"DirectX12/resource/constant_buffer.hpp"
#include<vector>
#include<array>
#include<string>


namespace DX12
{
	class device;
	class command_list;
	class resource;
	class depth_stencil_buffer;

	class mmd_model
	{

		//頂点
		vertex_buffer m_vertex_buffer{};
		//インデックス
		index_buffer m_index_buffer{};

		//テクスチャ
		//マテリアルによって添え字で指定される
		std::vector<texture_resource> m_texture{};

		//シーンのデータの定数バッファ
		//resource m_scene_constant_resource{};
		DX12::constant_buffer m_scene_constant_resource{};

		//マテリアル用の定数バッファ
		std::array<constant_buffer, 64> m_material_constant_resource{};

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
		unsigned int m_all_index_num{};

		//ディスクリプタヒープ
		//SetDescriptorHeapsを複数回呼び出すとうまくいかなかったのであきらめて1つにまとめた
		descriptor_heap<descriptor_heap_type::CBV_SRV_UAV> m_descriptor_heap{};

		//マテリアルごとのgpuハンドルの先頭
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> m_matarial_root_gpu_handle{};

		//とりあえず白と黒のテクスチャもメンバとして持たせる
		white_texture_resource m_white_texture_resource{};
		black_texture_resource m_black_texture_resource{};
		//グラデーションも
		gray_gradation_texture_resource m_gray_gradation_texture_resource{};

		//lightDepthのビューのハンドルのメモ
		D3D12_GPU_DESCRIPTOR_HANDLE m_light_depth_gpu_handle{};

	public:
		//コマンドリストはテクスチャのコピー用
		bool initialize(device*, const MMDL::pmx_model<std::wstring>&, command_list*, depth_stencil_buffer* lightDepthResource);

		void draw(command_list* cl);

		void map_scene_data(const scene_data&);

		//ライト深度バッファへの描写
		void draw_light_depth(command_list* cl);

	};

}