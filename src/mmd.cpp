#include"mmd.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/index_buffer.hpp"
#include"DirectX12/constant_buffer_resource.hpp"
#include"DirectX12/utility.hpp"
#include"DirectX12/texture_shader_resource.hpp"
#include"DirectX12/pipeline_state.hpp"
#include"DirectX12/descriptor_heap.hpp"

namespace ichi
{


	bool my_mmd::initialize(device* device, std::vector<my_vertex>& vertex,std::vector<unsigned short>& index, 
		std::vector<my_material>& material, std::vector<my_material_info>& materialInfo,std::vector<std::wstring>& filePath,command_list* cl)
	{
		//頂点
		m_vertex_buffer = std::unique_ptr<vertex_buffer>{
			device->create<vertex_buffer>(vertex.size() * sizeof(vertex[0]), sizeof(vertex[0]))
		};
		if (!m_vertex_buffer) {
			std::cout << "mmd v is failed\n";
			return false;
		}
		m_vertex_buffer->map(vertex);

		//インデックス
		m_index_buffer = std::unique_ptr<index_buffer>{
			device->create<index_buffer>(index.size() * sizeof(index[0]))
		};
		if (!m_index_buffer) {
			std::cout << "mmd i is failed\n";
			return false;
		}
		m_index_buffer->map(index);

		//一応チェック
		//サイズは同じになるはず
		assert(material.size() == materialInfo.size());

		//マテリアル
		for (auto& m : material) {
			auto ptr = device->create<constant_buffer_resource>(sizeof(material[0]));
			if (!ptr) {
				std::cout << "mmd material is failed\n";
				return false;
			}
			//仮
			ptr->map(std::vector{ m });
			m_material_resource.emplace_back(std::unique_ptr<constant_buffer_resource>(ptr));
		}

		//マテリアルの情報
		m_material_info = materialInfo;

		for (auto& path : filePath)
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

		m_world_mat_resource = std::unique_ptr<ichi::constant_buffer_resource>{
			device->create<ichi::constant_buffer_resource>(sizeof(DirectX::XMMATRIX))
		};
		if (!m_world_mat_resource) {
			std::cout << "mmd w mat is failed\n";
			return false;
		}
		m_viewproj_mat_resource= std::unique_ptr<ichi::constant_buffer_resource>{
			device->create<ichi::constant_buffer_resource>(sizeof(DirectX::XMMATRIX))
		};
		if (!m_viewproj_mat_resource) {
			std::cout << "mmd vp mat is failed\n";
			return false;
		}

		return true;
	}

	void my_mmd::draw_command(command_list* cl, descriptor_heap* dh, device* d,unsigned int i)
	{
		
		unsigned int indexOffset = 0;

		for (int k = 0; k < i; k++)
			indexOffset += m_material_info[k].m_vertex_num;

		
		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer->get_view());
		cl->get()->IASetIndexBuffer(&m_index_buffer->get_view());

		dh->reset();
		dh->create_view(d, m_world_mat_resource.get());
		dh->create_view(d, m_viewproj_mat_resource.get());
		dh->create_view(d, m_material_resource[i].get());
		dh->create_view(d, m_texture[m_material_info[i].m_texture_index].get());

		cl->get()->SetDescriptorHeaps(1, &dh->get());
		cl->get()->SetGraphicsRootDescriptorTable(0, dh->get()->GetGPUDescriptorHandleForHeapStart());

		cl->get()->DrawIndexedInstanced(m_material_info[i].m_vertex_num, 1, indexOffset, 0, 0);
		

	}

	unsigned int my_mmd::get_material_num()
	{
		return m_material_resource.size();
	}

	void my_mmd::map_world_mat(DirectX::XMMATRIX& mat)
	{
		map_func(m_world_mat_resource.get()->get(), mat);
	}

	void my_mmd::map_viewproj_mat(DirectX::XMMATRIX& mat)
	{
		map_func(m_viewproj_mat_resource.get()->get(), mat);
	}



}