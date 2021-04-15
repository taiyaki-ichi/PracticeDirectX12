#include"mmd_model.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/shader.hpp"
#include"window_size.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/utility.hpp"
#include"DirectX12/resource/depth_stencil_buffer.hpp"
#include<algorithm>
#include<iterator>
#include<utility>
#include<cassert>
#include<array>

namespace DX12
{
	namespace
	{
		//シャドウマップ用の深度バッファ生成時に使用


		//頂点のmap用の構造体
		struct map_vertex
		{
			//頂点座標
			DirectX::XMFLOAT3 m_position{};
			//法線
			DirectX::XMFLOAT3 m_normal{};
			//UV座標
			DirectX::XMFLOAT2 m_uv{};

			//ボーンの種類のフラグ
			unsigned char m_bone_type_flag{};
			//ボーン番号たち
			std::array<unsigned short, 4> m_bone_no{};
			//ウェイトたち
			std::array<float, 4> m_weight{};

			//SDEF用
			std::array<DirectX::XMFLOAT3, 3> m_SDEF{};
		};



		//mapvertexの生成
		std::vector<map_vertex> generate_map_vertex(const std::vector<MMDL::pmx_vertex>& vertex)
		{
			std::vector<map_vertex> result{};

			auto func = [](const MMDL::pmx_vertex& v) -> map_vertex {
				return { v.m_position,v.m_normal,v.m_uv ,static_cast<unsigned char>(v.m_bone_type_flag),v.m_bone,v.m_weight,v.m_SDEF_vector };
			};

			std::transform(vertex.begin(), vertex.end(), std::back_inserter(result), std::move(func));

			return result;
		}


		//インデックスの生成
		std::vector<unsigned short> generate_map_index(const std::vector<MMDL::pmx_surface>& index) 
		{
			std::vector<unsigned short> result{};

			for (auto tmp : index) {
				result.emplace_back(tmp.m_vertex_index);
			}

			return result;
		}

		//リソースにマップする用の構造体
		struct map_material {
			//MMDLのmaterialは3じゃあなくて4だった
			//とりあえずはFLOAT3でやる
			DirectX::XMFLOAT4 m_diffuse;
			DirectX::XMFLOAT3 m_specular;
			float m_specularity;
			DirectX::XMFLOAT3 m_ambient;
		};

		//map_materialの生成
		template<typename StringType>
		inline std::vector<map_material> generate_map_material(const std::vector<MMDL::pmx_material<StringType>>& material)
		{

			std::vector<map_material> result{};

			auto func = [](const auto& m) -> map_material {
				return { m.m_diffuse,m.m_specular,m.m_specularity,m.m_ambient };
			};

			std::transform(material.begin(), material.end(), std::back_inserter(result), std::move(func));

			return result;
		}

		//再帰用
		//とりあえずO(n^2)
		BoneNode get_bone_node_impl(BoneNode&& rootBoneNode, const MMDL::pmx_model<std::wstring>& pmxModel)
		{
			for (std::size_t i = 0; i < pmxModel.m_bone.size(); i++)
			{
				if (pmxModel.m_bone[i].m_parent_index == rootBoneNode.m_bone_index) {
					BoneNode childrenBoneNode{};
					childrenBoneNode.m_bone_index = i;
					childrenBoneNode.m_position = pmxModel.m_bone[i].m_position;
					rootBoneNode.m_children.emplace_back(get_bone_node_impl(std::move(childrenBoneNode), pmxModel));
				}
			}
			return rootBoneNode;
		}

		BoneNode get_bone_node(const MMDL::pmx_model<std::wstring>& pmxModel)
		{
			//まず親のボーンの取得
			BoneNode rootBoneNode{};
			for (std::size_t i = 0; i < pmxModel.m_bone.size(); i++)
			{
				if (pmxModel.m_bone[i].m_name.compare(0, 4, L"全ての親") == 0)
				{
					rootBoneNode.m_bone_index = i;
					rootBoneNode.m_position = pmxModel.m_bone[i].m_position;
					break;
				}
			}

			return get_bone_node_impl(std::move(rootBoneNode), pmxModel);
		}


	}


	bool mmd_model::initialize(device* device,const MMDL::pmx_model<std::wstring>& pmxModel,command_list* cl, depth_stencil_buffer* lightDepthResource)
	{

		//頂点
		auto vertex = generate_map_vertex(pmxModel.m_vertex);
		m_vertex_buffer.initialize(device, static_cast<unsigned int>(vertex.size() * sizeof(vertex[0])),static_cast<unsigned int>(sizeof(vertex[0])));
		if (m_vertex_buffer.is_empty()) {
			std::cout << "mmd　model v is failed\n";
			return false;
		}
		m_vertex_buffer.map(std::move(vertex));


		//インデックス
		m_all_index_num = static_cast<unsigned int>(pmxModel.m_surface.size());
		auto index = generate_map_index(pmxModel.m_surface);
		m_index_buffer.initialize(device, static_cast<unsigned int>(index.size() * sizeof(index[0])));
		if (m_index_buffer.is_empty()) {
			std::cout << "mmd model i is failed\n";
			return false;
		}
		m_index_buffer.map(std::move(index));


		//マテリアル
		auto material = generate_map_material(pmxModel.m_material);
		for (int i = 0; i < material.size(); i++)
		{

			m_material_constant_resource[i].initialize(device, sizeof(material[i]));
			if (m_material_constant_resource[i].is_empty()) {
				std::cout << "mmd material is failed\n";
				return false;
			}
			map_material data[] = { material[i] };
			m_material_constant_resource[i].map(data);
		}
		
		//マテリアルインフォ
		auto func = [](const MMDL::pmx_material<std::wstring>& m) -> material_info {
			return { m.m_texture_index_size_1,m.m_texture_index_size_2,m.m_vertex_num };
		};
		std::transform(pmxModel.m_material.begin(), pmxModel.m_material.end(), std::back_inserter(m_material_info), std::move(func));


		//テクスチャ
		for (auto& path : pmxModel.m_texture_path)
		{
			auto imageResult = get_texture(path.c_str());
			if (!imageResult) {
				std::cout << "image si failed\n";
				return false;
			}
			auto& [metaData, scratchImage] = imageResult.value();

			DX12::texture_resource tmp{};
			tmp.initialize(device, cl, &metaData, &scratchImage);
			if (tmp.is_empty()) {
				std::cout << "mmd texture init is failed\n";
				return false;
			}
			m_texture.emplace_back(std::move(tmp));
		}

		//シーンデータ
		m_scene_constant_resource.initialize(device, sizeof(scene_data));
		if (m_scene_constant_resource.is_empty()) {
			std::cout << "scene data constant init is failed\n";
			return false;
		}

		//座標移動系
		//とりあえずワールドのみ
		m_transform_constant_resource.initialize(device, sizeof(transform_data));
		if (m_transform_constant_resource.is_empty())
			throw "";
		

		m_white_texture_resource.initialize(device);
		if (m_white_texture_resource.is_empty()) {
			std::cout << "mmd whire tex is falied\n";
			return false;
		}

		m_black_texture_resource.initialize(device);
		if (m_black_texture_resource.is_empty()) {
			std::cout << "mmd black tex is failed\n";
			return false;
		}

		m_gray_gradation_texture_resource.initialize(device);
		if (m_gray_gradation_texture_resource.is_empty()) {
			std::cout << "mmd gray grade is failed\n";
			return false;
		}

	
		//今のところ個数は行列1つとマテリアル分
		//ライト深度ようにもう1つ
		if (!m_descriptor_heap.initialize(device, static_cast<unsigned int>(2 + m_material_info.size() * 5 + 1))) {
			std::cout << "mmd desc heap is failed\n";
			return false;
		}

		m_descriptor_heap.create_view(device, &m_scene_constant_resource);
		m_descriptor_heap.create_view(device, &m_transform_constant_resource);


		for (size_t i = 0; i < m_material_info.size(); i++)
		{
			//先頭のハンドルはメモしておく
			auto handle = m_descriptor_heap.create_view(device, &m_material_constant_resource[i]);
			if (handle)
				m_matarial_root_gpu_handle.emplace_back(handle.value().first);
			else {
				std::cout << "mmd desc heap create view is failed\n";
				return false;
			}

			//有効なテクスチャがある場合
			if (0 <= m_material_info[i].m_texture_index && m_material_info[i].m_texture_index < m_texture.size())
				m_descriptor_heap.create_view(device, &m_texture[m_material_info[i].m_texture_index]);
			//ない場合は白テクスチャ
			else
				m_descriptor_heap.create_view(device, &m_white_texture_resource);


			//加算スフィア
			if (pmxModel.m_material[i].m_sphere_mode == 2)
				m_descriptor_heap.create_view(device, &m_texture[m_material_info[i].m_toon_index]);
			else
				m_descriptor_heap.create_view(device, &m_black_texture_resource);

			//乗算スフィア
			if (pmxModel.m_material[i].m_sphere_mode == 1)
				m_descriptor_heap.create_view(device, &m_texture[m_material_info[i].m_toon_index]);
			else
				m_descriptor_heap.create_view(device, &m_white_texture_resource);

			//toon
			//個別toonなら対応
			const unsigned int* ptr = std::get_if<0>(&pmxModel.m_material[i].m_toon);
			if (ptr && *ptr < m_texture.size())
				m_descriptor_heap.create_view(device, &m_texture[*ptr]);
			else 
				m_descriptor_heap.create_view(device, &m_gray_gradation_texture_resource);
		}
		
		//ライト深度のリソースの描写用のViewを生成
		//ハンドルはメモしておく（おっふせっとからアクセスできるけど。。）
		{
			auto result = m_descriptor_heap.create_view(device, lightDepthResource);
			if (result)
				m_light_depth_gpu_handle = result.value().first;
			else {
				std::cout << "mmd init descriptor light shader failed\n";
				return false;
			}
		}

		//
		//ボーン
		//
		m_bone_matrices.resize(pmxModel.m_bone.size());
		std::fill(m_bone_matrices.begin(), m_bone_matrices.end(), DirectX::XMMatrixIdentity());

		m_bone_node = get_bone_node(pmxModel);

		return true;
	}

	void mmd_model::draw(command_list* cl)
	{
		
		unsigned int indexOffset = 0;

		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer.get_view());
		cl->get()->IASetIndexBuffer(&m_index_buffer.get_view());

		cl->get()->SetDescriptorHeaps(1, &m_descriptor_heap.get());

		cl->get()->SetGraphicsRootDescriptorTable(0, m_descriptor_heap.get_gpu_handle());

		
		cl->get()->SetGraphicsRootDescriptorTable(2, m_light_depth_gpu_handle);

	
		for (size_t i = 0; i < m_material_info.size(); i++)
		{

			cl->get()->SetGraphicsRootDescriptorTable(1, m_matarial_root_gpu_handle[i]);

			cl->get()->DrawIndexedInstanced(m_material_info[i].m_vertex_num, 2, indexOffset, 0, 0);

			indexOffset += m_material_info[i].m_vertex_num;
		
		}
		
	}

	void mmd_model::map_scene_data(const scene_data& sd)
	{
		scene_data* ptr = nullptr;
		m_scene_constant_resource.get()->Map(0, nullptr, (void**)&ptr);

		*ptr = sd;

		m_scene_constant_resource.get()->Unmap(0, nullptr);
	}

	void mmd_model::map_transform_data(const transform_data& td)
	{
		transform_data* ptr = nullptr;
		m_transform_constant_resource.get()->Map(0, nullptr, (void**)&ptr);

		*ptr = td;

		m_transform_constant_resource.get()->Unmap(0, nullptr);
	}

	void mmd_model::draw_light_depth(command_list* cl)
	{

		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer.get_view());
		cl->get()->IASetIndexBuffer(&m_index_buffer.get_view());

		cl->get()->SetDescriptorHeaps(1, &m_descriptor_heap.get());
		cl->get()->SetGraphicsRootDescriptorTable(0, m_descriptor_heap.get_gpu_handle());

		cl->get()->DrawIndexedInstanced(m_all_index_num, 1, 0, 0, 0);
	}

}