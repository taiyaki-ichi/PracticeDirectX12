#include"perapolygon.hpp"
#include"DirectX12/device.hpp"
#include"perapolygon_helper_functions.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/resource_type_tag.hpp"
#include"DirectX12/descriptor_heap.hpp"

namespace ichi
{

	perapolygon::~perapolygon()
	{
		if (m_pipeline_state)
			m_pipeline_state->Release();
		if (m_root_signature)
			m_root_signature->Release();
		if (m_color_resource)
			m_color_resource->Release();
		if (m_normal_resource)
			m_normal_resource->Release();
	}

	bool perapolygon::initialize(device* device)
	{
		//ルートシグネチャ
		{
			auto result = create_perapolygon_root_signature(device);
			if (result)
				m_root_signature = result.value();
			else {
				std::cout << "pera root sig is failed\n";
				return false;
			}
		}

		//パイプインステート
		{
			auto result = create_perapolygon_pipline_state(device, m_root_signature);
			if (result)
				m_pipeline_state = result.value();
			else {
				std::cout << "pera pipe line is failed\n";
				return false;
			}
		}

		//実際のリソース
		{
			auto result = create_perapolygon_resource(device);
			if (result) {
				m_color_resource = result.value().first;
				m_normal_resource = result.value().second;
			}
			else {
				std::cout << "pera resource is failed\n";
				return false;
			}
		}

		//レンダーターゲット用の
		{
			m_rtv_descriptor_heap = std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>>{
				device->create<descriptor_heap<descriptor_heap_type::RTV>>(2)
			};
			if (!m_rtv_descriptor_heap) {
				std::cout << "pera rtv descriptor heap is failed\n";
				return false;
			}
		}

		//シェーダリソースの
		{
			m_cbv_srv_usv_descriptor_heap = std::unique_ptr<descriptor_heap<descriptor_heap_type::CBV_SRV_UAV>>{
				device->create<descriptor_heap<descriptor_heap_type::CBV_SRV_UAV>>(2)
			};
			if (!m_cbv_srv_usv_descriptor_heap) {
				std::cout << "pera srv descriptoe heap is failed\n";
				return false;
			}
		}

		//頂点バッファ
		{
			auto result = create_perapolygon_vertex_buffer(device);
			if (result)
				m_vertex_buffer = std::move(result.value());
			else {
				std::cout << "pera vert buffer is failed\n";
				return false;
			}
		}

		//レンダーターゲット用のディスクリプタヒープにViewを作製
		{
			auto result = m_rtv_descriptor_heap->create_view<create_view_type::RTV>(device, m_color_resource);
			if (result)
				m_render_target_view_cpu_handle_array[0] = result.value().second;
			else {
				std::cout << "pera rtv create view 1 is faield \n";
				return false;
			}
		}
		{
			auto result = m_rtv_descriptor_heap->create_view<create_view_type::RTV>(device, m_normal_resource);
			if (result)
				m_render_target_view_cpu_handle_array[1] = result.value().second;
			else {
				std::cout << "pera rtv create view 2 is failed\n";
				return false;
			}
		}

		
		//シェーダリソース用のディスクリプタヒープにViewを作製
		{
			auto result = m_cbv_srv_usv_descriptor_heap->create_view<create_view_type::SRV>(device, m_color_resource);
			if (!result) {
				std::cout << "pera srv create view is afled\n";
				return false;
			}
		}
		{
			auto result = m_cbv_srv_usv_descriptor_heap->create_view<create_view_type::SRV>(device, m_normal_resource);
			if (!result) {
				std::cout << "pera srv create view is failed\n";
				return false;
			}
		}
		

		return true;
	}

	std::pair<int, D3D12_CPU_DESCRIPTOR_HANDLE*> perapolygon::get_render_target_info()
	{
		return std::make_pair(2, m_render_target_view_cpu_handle_array);
	}

	void perapolygon::begin_drawing_resource_barrier(command_list* cl)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_color_resource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		BarrierDesc.Transition.pResource = m_normal_resource;
		cl->get()->ResourceBarrier(1, &BarrierDesc);
	}

	void perapolygon::end_drawing_resource_barrier(command_list* cl)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_color_resource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		BarrierDesc.Transition.pResource = m_normal_resource;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

	}

	void perapolygon::clear(command_list* cl)
	{
		float clearColor[] = { 0.5f,0.5f,0.5f,1.0f };
		//レンダーターゲットじゃあなくてシェーダリソースのディスクリプタからハンドルを取得しても行けそうだけど。。。
		//どうなんかな
		cl->get()->ClearRenderTargetView(m_rtv_descriptor_heap->get_cpu_handle(0), clearColor, 0, nullptr);
		cl->get()->ClearRenderTargetView(m_rtv_descriptor_heap->get_cpu_handle(1), clearColor, 0, nullptr);
	}

	void perapolygon::draw(command_list* cl)
	{
		cl->get()->SetPipelineState(m_pipeline_state);
		cl->get()->SetGraphicsRootSignature(m_root_signature);

		cl->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer->get_view());
		cl->get()->SetDescriptorHeaps(1, &m_cbv_srv_usv_descriptor_heap->get());
		cl->get()->SetGraphicsRootDescriptorTable(0, m_cbv_srv_usv_descriptor_heap->get_gpu_handle());
		cl->get()->DrawInstanced(4, 1, 0, 0);

	}

}