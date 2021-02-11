#include"mmd_model.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/index_buffer.hpp"
#include"DirectX12/constant_buffer_resource.hpp"
#include"DirectX12/texture_shader_resource.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/color_texture.hpp"
#include"DirectX12/shader.hpp"
#include"mmd_model_helper_functions.hpp"
#include"window_size.hpp"
#include<algorithm>
#include<iterator>
#include<utility>
#include<cassert>

namespace ichi
{
	namespace
	{
		//シャドウマップ用の深度バッファ生成時に使用
		constexpr uint32_t shadow_difinition = 1024;


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



	mmd_model::~mmd_model()
	{
		if (m_depth_resource)
			m_depth_resource->Release();
		if (m_light_depth_resource)
			m_light_depth_resource->Release();
		if (m_pipeline_state)
			m_pipeline_state->Release();
		if (m_shadow_pipeline_state)
			m_shadow_pipeline_state->Release();
		if (m_root_signature)
			m_root_signature->Release();

	}

	bool mmd_model::initialize(device* device,const MMDL::pmx_model<std::wstring>& pmxModel,command_list* cl)
	{


		{
			auto result = create_mmd_rootsignature(device);
			if (result)
				m_root_signature = result.value();
			else {
				std::cout << "mmd init root sig is failed\n";
				return false;
			}
		}

		{
			auto result = create_mmd_pipline_state(device, m_root_signature);
			if (result) {
				m_pipeline_state = result.value().first;
				m_shadow_pipeline_state = result.value().second;
			}
			else {
				std::cout << "mmd init pipe is failed \n";
				return false;
			}
		}


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
		m_all_index_num = pmxModel.m_surface.size();
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
		//ライト深度ようにもう1つ
		m_descriptor_heap = std::unique_ptr<descriptor_heap<descriptor_heap_type::CBV_SRV_UAV>>{
			device->create<descriptor_heap<descriptor_heap_type::CBV_SRV_UAV>>(1 + m_material_info.size() * 5 + 1)
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
				m_matarial_root_gpu_handle.emplace_back(handle.value().first);
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


		{
			auto result = create_mmd_depth_buffers(device, window_width, window_height, shadow_difinition);
			if (result) {
				m_depth_resource = result.value().first;
				m_light_depth_resource = result.value().second;
			}
			else {
				std::cout << "create mmd depth bu is failed\n";
				return false;
			}
		}
		
		{
			m_depth_descriptor_heap = std::unique_ptr<descriptor_heap<descriptor_heap_type::DSV>>{
				device->create<descriptor_heap<descriptor_heap_type::DSV>>(2)//普通のとライト用の2つ
			};
			if (!m_depth_descriptor_heap) {
				std::cout << "mmd depth desc heap is failed\n";
				return false;
			}
		}

		{
			auto result = m_depth_descriptor_heap->create_view<create_view_type::DSV>(device, m_depth_resource);
			if (!result){
				std::cout << "mmd create view depth is faield\n";
				return false;
			}
		}

		{
			auto result = m_depth_descriptor_heap->create_view<create_view_type::DSV>(device, m_light_depth_resource);
			if (!result) {
				std::cout << "mmd create view light depth failed \n";
				return false;
			}
		}
		
		{
			auto result = m_descriptor_heap->create_view<create_view_type::DSV>(device, m_light_depth_resource);
			if (result)
				m_light_depth_gpu_handle = result.value().first;
			else {
				std::cout << "mmd init descriptor light shader failed\n";
				return false;
			}
		}
		return true;
	}

	void mmd_model::draw(command_list* cl)
	{
		
		unsigned int indexOffset = 0;
	
		cl->get()->SetPipelineState(m_pipeline_state);
		cl->get()->SetGraphicsRootSignature(m_root_signature);

		cl->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer->get_view());
		cl->get()->IASetIndexBuffer(&m_index_buffer->get_view());

		cl->get()->SetDescriptorHeaps(1, &m_descriptor_heap->get());

		cl->get()->SetGraphicsRootDescriptorTable(0, m_descriptor_heap->get_gpu_handle());

		
		cl->get()->SetGraphicsRootDescriptorTable(2, m_light_depth_gpu_handle);

	
		for (int i = 0; i < m_material_info.size(); i++)
		{

			cl->get()->SetGraphicsRootDescriptorTable(1, m_matarial_root_gpu_handle[i]);

			cl->get()->DrawIndexedInstanced(m_material_info[i].m_vertex_num, 2, indexOffset, 0, 0);

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

	void mmd_model::draw_light_depth(command_list* cl)
	{
		//ここは先頭のハンドルで大丈夫かな
		cl->get()->ClearDepthStencilView(m_depth_descriptor_heap->get_cpu_handle(1),
			D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		D3D12_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(shadow_difinition);//出力先の幅(ピクセル数)
		viewport.Height = static_cast<float>(shadow_difinition);//出力先の高さ(ピクセル数)
		viewport.TopLeftX = 0;//出力先の左上座標X
		viewport.TopLeftY = 0;//出力先の左上座標Y
		viewport.MaxDepth = 1.0f;//深度最大値
		viewport.MinDepth = 0.0f;//深度最小値
		cl->get()->RSSetViewports(1, &viewport);

		D3D12_RECT scissorrect{};
		scissorrect.top = 0;//切り抜き上座標
		scissorrect.left = 0;//切り抜き左座標
		scissorrect.right = scissorrect.left + shadow_difinition;//切り抜き右座標
		scissorrect.bottom = scissorrect.top + shadow_difinition;//切り抜き下座標
		cl->get()->RSSetScissorRects(1, &scissorrect);


		cl->get()->SetPipelineState(m_shadow_pipeline_state);
		cl->get()->SetGraphicsRootSignature(m_root_signature);

		cl->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer->get_view());
		cl->get()->IASetIndexBuffer(&m_index_buffer->get_view());

		cl->get()->SetDescriptorHeaps(1, &m_descriptor_heap->get());
		cl->get()->SetGraphicsRootDescriptorTable(0, m_descriptor_heap->get_gpu_handle());
		cl->get()->SetGraphicsRootDescriptorTable(2, m_light_depth_gpu_handle);

		auto handle = m_depth_descriptor_heap->get_cpu_handle(1);
		cl->get()->OMSetRenderTargets(0, nullptr, false, &handle);

		cl->get()->DrawIndexedInstanced(m_all_index_num, 1, 0, 0, 0);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE mmd_model::get_depth_resource_cpu_handle() const noexcept
	{
		return m_depth_descriptor_heap->get_cpu_handle(0);
	}

	ID3D12Resource* mmd_model::get_depth_resource() noexcept
	{
		return m_depth_resource;
	}

}