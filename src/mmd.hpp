#pragma once
#include"DirectX12/texture_shader_resource.hpp"
#include<memory>
#include<vector>
#include<DirectXMath.h>

namespace ichi
{

	class vertex_buffer;
	class index_buffer;
	class constant_buffer_resource;
	class device;
	class command_list;
	class descriptor_heap;
	class pipeline_state;


	//pmx_vertexから必要な情報を抽出
	//随時、更新していく
	struct my_vertex
	{
		//頂点座標
		DirectX::XMFLOAT3 m_position{};
		//法線
		DirectX::XMFLOAT3 m_normal{};
		//UV座標
		DirectX::XMFLOAT2 m_uv{};
	};

	struct my_material {
		//MMDLのmaterialは3じゃあなくて4だった
		//とりあえずはFLOAT3でやる
		DirectX::XMFLOAT4 m_diffuse;
		DirectX::XMFLOAT3 m_specular;
		float m_specularity;
		DirectX::XMFLOAT3 m_ambient;
	};

	struct my_material_info {
		//m_textureのサイズより多いな値は無効
		unsigned int m_texture_index = 0;
		unsigned int m_toon_index = 0;
		//頂点を指すインデックスの数
		int m_vertex_num = 0;
	};

	//mmdのクラス
	class my_mmd
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
		std::unique_ptr<constant_buffer_resource> m_world_mat_resource{};
		std::unique_ptr<constant_buffer_resource> m_viewproj_mat_resource{};

		//今のところfloat4,float3,float,float3のmy_material
		std::vector<std::unique_ptr<constant_buffer_resource>> m_material_resource{};
		
		//m_material_resourceに対応した添え字
		std::vector<my_material_info> m_material_info{};
		
	public:
		my_mmd() = default;
		~my_mmd() = default;

		//とりあえずwstringだけ
		bool initialize(device*, std::vector<my_vertex>&, std::vector<unsigned short>& index,
			std::vector<my_material>&, std::vector<my_material_info>&, std::vector<std::wstring>& filePath,
			command_list*);

		//DoubleBufferのbegin、プリミティブ、シザー矩形、ビューポートを設定した後呼び出す
		//i番目のマテリアルの描写
		void draw_command(command_list*, descriptor_heap*, device*, unsigned int i);

		//アテリアルの数を返す
		unsigned int get_material_num();

		void map_world_mat(DirectX::XMMATRIX&);
		void map_viewproj_mat(DirectX::XMMATRIX&);


	};

}