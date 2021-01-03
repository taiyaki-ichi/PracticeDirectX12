#include"mmd_model.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/index_buffer.hpp"
#include"DirectX12/constant_buffer_resource.hpp"
#include"DirectX12/texture_shader_resource.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/color_texture.hpp"
#include<algorithm>
#include<iterator>
#include<utility>
#include<cassert>

namespace ichi
{
	namespace
	{
		//頂点のmap用の構造体
		struct map_vertex
		{
			//頂点座標
			DirectX::XMFLOAT3 m_position{};
			//法線
			DirectX::XMFLOAT3 m_normal{};
			//UV座標
			DirectX::XMFLOAT2 m_uv{};
		};

		//mapvertexの生成
		std::vector<map_vertex> generate_map_vertex(const std::vector<MMDL::pmx_vertex>& vertex)
		{
			std::vector<map_vertex> result{};

			auto func = [](const MMDL::pmx_vertex& v) -> map_vertex {
				return { v.m_position,v.m_normal,v.m_uv };
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





	}



	bool mmd_model::initialize(device* device,const MMDL::pmx_model<std::wstring>& pmxModel,command_list* cl)
	{
		//頂点
		auto vertex = generate_map_vertex(pmxModel.m_vertex);
		m_vertex_buffer = std::unique_ptr<vertex_buffer>{
			device->create<vertex_buffer>(vertex.size() * sizeof(vertex[0]), sizeof(vertex[0]))
		};
		if (!m_vertex_buffer) {
			std::cout << "mmd　model v is failed\n";
			return false;
		}
		m_vertex_buffer->map(std::move(vertex));


		//インデックス
		auto index = generate_map_index(pmxModel.m_surface);
		m_index_buffer = std::unique_ptr<index_buffer>{
			device->create<index_buffer>(index.size() * sizeof(index[0]))
		};
		if (!m_index_buffer) {
			std::cout << "mmd model i is failed\n";
			return false;
		}
		m_index_buffer->map(std::move(index));


		//マテリアル
		auto material = generate_map_material(pmxModel.m_material);
		for (auto& m : material) {
			auto ptr = device->create<constant_buffer_resource>(sizeof(m));
			if (!ptr) {
				std::cout << "mmd material is failed\n";
				return false;
			}
			//
			//仮
			//
			map_material tmp[] = { m };
			ptr->map(tmp);
			m_material_resource.emplace_back(std::unique_ptr<constant_buffer_resource>(ptr));
		}
		
		//マテリアルインフォ
		auto func = [](const MMDL::pmx_material<std::wstring>& m) -> material_info {
			return { m.m_texture_index_size_1,m.m_texture_index_size_2,m.m_vertex_num };
		};
		std::transform(pmxModel.m_material.begin(), pmxModel.m_material.end(), std::back_inserter(m_material_info), std::move(func));


		//テクスチャ
		for (auto& path : pmxModel.m_texture_path)
		{
			auto imageResult = ichi::get_texture(path.c_str());
			if (!imageResult) {
				std::cout << "image si failed\n";
				return false;
			}
			auto& [metaData, scratchImage] = imageResult.value();

			auto textureBuffer = device->create<texture_shader_resource>(&metaData, &scratchImage);
			auto uploadTextureBuffer = device->create<upload_texture_shader_resource>(&metaData, &scratchImage);

			uploadTextureBuffer->map(*scratchImage.GetImage(0, 0, 0));

			cl->excute_copy_texture(uploadTextureBuffer, textureBuffer);

			delete uploadTextureBuffer;
			m_texture.emplace_back(std::unique_ptr<texture_shader_resource>{textureBuffer});
		}

		//シーンデータ
		m_scene_data_resource = std::unique_ptr<ichi::constant_buffer_resource>{
			device->create<constant_buffer_resource>(sizeof(scene_data))
		};
		if (!m_scene_data_resource) {
			std::cout << "mmd scene data is failed\n";
			return false;
		}

		m_white_texture_resource = std::unique_ptr<white_texture_resource>{
			device->create<white_texture_resource>()
		};
		if (!m_white_texture_resource) {
			std::cout << "mmd whire tex is falied\n";
			return false;
		}

		m_black_texture_resource = std::unique_ptr<black_texture_resource>{
			device->create<black_texture_resource>()
		};
		if (!m_black_texture_resource) {
			std::cout << "mmd black tex is failed\n";
			return false;
		}

		m_gray_gradation_texture_resource = std::unique_ptr<gray_gradation_texture_resource>{
			device->create<gray_gradation_texture_resource>()
		};
		if (!m_gray_gradation_texture_resource) {
			std::cout << "mmd gray grade is failed\n";
			return false;
		}

	
		//今のところ個数は行列1つとマテリアル分
		m_descriptor_heap = std::unique_ptr<descriptor_heap>{
			device->create<descriptor_heap>(1 + m_material_info.size() * 5)
		};
		if (!m_descriptor_heap) {
			std::cout << "mmd desc heap is failed\n";
			return false;
		}


		m_descriptor_heap->create_view(device, m_scene_data_resource.get());

		for (int i = 0; i < m_material_info.size(); i++)
		{
			//先頭のハンドルはメモしておく
			auto handle = m_descriptor_heap->create_view(device, m_material_resource[i].get());
			if (handle)
				m_matarial_root_gpu_handle.emplace_back(handle.value());
			else {
				std::cout << "mmd desc heap create view is failed\n";
				return false;
			}

			//有効なテクスチャがある場合
			if (0 <= m_material_info[i].m_texture_index && m_material_info[i].m_texture_index < m_texture.size())
				m_descriptor_heap->create_view(device, m_texture[m_material_info[i].m_texture_index].get());
			//ない場合は白テクスチャ
			else
				m_descriptor_heap->create_view(device, m_white_texture_resource.get());


			//加算スフィア
			if (pmxModel.m_material[i].m_sphere_mode == 2)
				m_descriptor_heap->create_view(device, m_texture[m_material_info[i].m_toon_index].get());
			else
				m_descriptor_heap->create_view(device, m_black_texture_resource.get());

			//乗算スフィア
			if (pmxModel.m_material[i].m_sphere_mode == 1)
				m_descriptor_heap->create_view(device, m_texture[m_material_info[i].m_toon_index].get());
			else
				m_descriptor_heap->create_view(device, m_white_texture_resource.get());

			//toon
			//個別toonなら対応
			const unsigned int* ptr = std::get_if<0>(&pmxModel.m_material[i].m_toon);
			if (ptr && *ptr < m_texture.size())
				m_descriptor_heap->create_view(device, m_texture[*ptr].get());
			else 
				m_descriptor_heap->create_view(device, m_gray_gradation_texture_resource.get());
		}



		return true;
	}

	void mmd_model::draw(command_list* cl)
	{
		unsigned int indexOffset = 0;
	
		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer->get_view());
		cl->get()->IASetIndexBuffer(&m_index_buffer->get_view());

		cl->get()->SetDescriptorHeaps(1, &m_descriptor_heap->get_ptr());
		cl->get()->SetGraphicsRootDescriptorTable(0, m_descriptor_heap->get_gpu_handle());

		for (int i = 0; i < m_material_info.size(); i++)
		{

			cl->get()->SetGraphicsRootDescriptorTable(1, m_matarial_root_gpu_handle[i]);

			cl->get()->DrawIndexedInstanced(m_material_info[i].m_vertex_num, 1, indexOffset, 0, 0);

			indexOffset += m_material_info[i].m_vertex_num;
		
		}

	
	}

	void mmd_model::map_scene_data(const scene_data& sd)
	{
		scene_data* ptr = nullptr;
		m_scene_data_resource->get()->Map(0, nullptr, (void**)&ptr);

		*ptr = sd;

		m_scene_data_resource->get()->Unmap(0, nullptr);
	}

}