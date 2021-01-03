#pragma once
#include"include/pmx_data_struct.hpp"
#include"DirectX12/texture_shader_resource.hpp"
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


	class mmd_model
	{
		//頂点
		std::unique_ptr<vertex_buffer> m_vertex_buffer{};
		//インデックス
		std::unique_ptr<index_buffer> m_index_buffer{};

		//テクスチャ
		//マテリアルによって添え字で指定される
		std::vector<std::unique_ptr<texture_shader_resource>> m_texture{};

		//シェーダに送る用
		//とりあえず、メンバとして保持しておく
		//使いまわれるようなら描写時の引数で十分
		std::unique_ptr<constant_buffer_resource> m_world_mat_resource{};
		std::unique_ptr<constant_buffer_resource> m_viewproj_mat_resource{};

		//今のところfloat4,float3,float,float3のmaterial
		//マテリアルの描写毎に定数バッファをリセットしビューを作製
		std::vector<std::unique_ptr<constant_buffer_resource>> m_material_resource{};

		//マテリアルの情報保持用
		struct material_info {
			//m_textureのサイズより多いな値は無効
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

	public:
		mmd_model() = default;
		~mmd_model() = default;

		//コマンドリストはテクスチャのコピー用
		bool initialize(device*,const MMDL::pmx_model<std::wstring>&,command_list*);


		//
		//仮
		//drawはコマンドリストのメンバ関数にしたいな
		//
		void draw(command_list* cl);


		void map_world_mat(DirectX::XMMATRIX&);
		void map_viewproj_mat(DirectX::XMMATRIX&);


	};

}