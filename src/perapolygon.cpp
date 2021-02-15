#include"perapolygon.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/command_list.hpp"
#include"window_size.hpp"
#include"scene_data.hpp"
#include"DirectX12/resource/depth_stencil_buffer.hpp"

namespace DX12
{


	bool perapolygon::initialize(device* device, depth_stencil_buffer* depthResource)
	{

		//レンダーターゲット用のディスクリプタヒープ
		if (!m_rtv_descriptor_heap.initialize(device, RESOURCE_NUM)) {
			std::cout << "pera rtv descriptor heap is failed\n";
			return false;
		}

		//シェーダリソースのディスクリプタヒープ
		//depthResourceののViewも作製するので+1
		if (!m_cbv_srv_usv_descriptor_heap.initialize(device, RESOURCE_NUM + 1)) {
			std::cout << "pera srv descriptoe heap is failed\n";
			return false;
		}

		//実際のリソースたちの初期化

		m_float4_resource[Float4ResourceIndex::COLOR].initialize(device, window_width, window_height,
			D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.5f,0.5f,0.5f,1.f } });

		m_float4_resource[Float4ResourceIndex::NORMAL].initialize(device, window_width, window_height,
			D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.5f,0.5f,0.5f,1.f } });

		m_float4_resource[Float4ResourceIndex::BLOOM].initialize(device, window_width, window_height,
			D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.f,0.f,0.f,1.f } });

		m_float4_resource[Float4ResourceIndex::SHRINK_BLOOM].initialize(device, window_width / 2, window_height,
			D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.f,0.f,0.f,1.f } });

		m_float4_resource[Float4ResourceIndex::DOF].initialize(device, window_width / 2, window_height,
			D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.f,0.f,0.f,1.f } });

		m_SSAO_resource.initialize(device, window_width, window_height, D3D12_CLEAR_VALUE{ DXGI_FORMAT_R32_FLOAT,{ 1.f,1.f,1.f,1.f } });

		m_constant_buffer.initialize(device, sizeof(scene_data_for_perapolygon));


		//頂点バッファ
		{
			struct PeraVert {
				DirectX::XMFLOAT3 pos;
				DirectX::XMFLOAT2 uv;
			};
			PeraVert pv[4] = {
				{{-1.f,-1.f,0.1f},{0,1.f}},
				{{-1.f,1.f,0.1f},{0,0}},
				{{1.f,-1.f,0.1f},{1.f,1.f}},
				{{1.f,1.f,0.1f},{1.f,0}},
			};

			if (!m_vertex_buffer.initialize(device, sizeof(pv), sizeof(pv[0]))) {
				std::cout << "pera vert buffer is failed\n";
				return false;
			}

			m_vertex_buffer.map(pv);
		}

		//レンダーターゲット用のディスクリプタヒープにViewを作製
		for (int i = 0; i < RESOURCE_NUM - 2; i++)
		{
			auto result = m_rtv_descriptor_heap.create_view(device, &m_float4_resource[i]);
			if (!result) {
				std::cout << "pera rtv create view " << i << " is failed\n";
				return false;
			}
		}
		//深度用としてSSAO用のViewを生成
		{
			auto result = m_rtv_descriptor_heap.create_view(device, &m_SSAO_resource);
			if (!result) {
				std::cout << "pera rtv crate view dsv is failed\n";
				return false;
			}
		}
		
	
		//レンダーターゲットに使用するハンドルをまとめておく
		for (int i = 0; i < RENDER_TARGET_HANDLE_NUM; i++)
			m_render_target_view_cpu_handle_array[i] = m_rtv_descriptor_heap.get_cpu_handle(i);


		//シェーダリソース用のディスクリプタヒープにViewを作製
		for (int i = 0; i < RESOURCE_NUM - 2; i++)
		{
			auto result = m_cbv_srv_usv_descriptor_heap.create_view(device, &m_float4_resource[i]);
			if (!result) {
				std::cout << "pera srv create view " << i << " is failed\n";
				return false;
			}
		}
		{
			auto result = m_cbv_srv_usv_descriptor_heap.create_view(device, &m_SSAO_resource);
			if (!result) {
				std::cout << "pera srv create view dsv is failed\n";
				return false;
			}
		}
	
		{
			auto result = m_cbv_srv_usv_descriptor_heap.create_view(device, &m_constant_buffer);
			if (!result) {
				std::cout << "pera srv create view cbv is failed\n";
				return false;
			}
		}

		//最後に深度バッファのViewを生成
		//つまり添え字はRESOURCE_NUM
		{
			auto result = m_cbv_srv_usv_descriptor_heap.create_view(device, depthResource);
			if (!result) {
				std::cout << "pera srv depth resource create view is failed\n";
				return false;
			}
		}


		return true;
	}

	std::pair<int, D3D12_CPU_DESCRIPTOR_HANDLE*> perapolygon::get_render_target_info()
	{
		return std::make_pair(RENDER_TARGET_HANDLE_NUM, m_render_target_view_cpu_handle_array);
	}


	void  perapolygon::all_resource_barrior(command_list* cl, D3D12_RESOURCE_STATES state)
	{
		for (int i = 0; i < RESOURCE_NUM - 2; i++)
			m_float4_resource[i].barrior(cl, state);
		m_SSAO_resource.barrior(cl, state);
	}

	void perapolygon::ssao_resource_barrior(command_list* cl, D3D12_RESOURCE_STATES state)
	{
		m_SSAO_resource.barrior(cl, state);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE perapolygon::get_ssao_cpu_handle()
	{
		//
		return m_rtv_descriptor_heap.get_cpu_handle(5);
	}

	void perapolygon::map_scene_data(const scene_data_for_perapolygon& sceneData)
	{
		scene_data_for_perapolygon* ptr = nullptr;
		m_constant_buffer.get()->Map(0, nullptr, (void**)&ptr);
		
		*ptr = sceneData;

		m_constant_buffer.get()->Unmap(0, nullptr);
	}

	void perapolygon::clear(command_list* cl)
	{
		float clearColor[] = { 0.5f,0.5f,0.5f,1.f };
		//ガウス補正で使うためクリアカラーは黒
		float clearColor2[] = { 0.f,0.f,0.f,1.f };
		float clearColor3[] = { 1.f,1.f,1.f,1.f };
		//レンダーターゲットじゃあなくてシェーダリソースのディスクリプタからハンドルを取得しても行けそうだけど。。。
		//どうなんかな
		cl->get()->ClearRenderTargetView(m_rtv_descriptor_heap.get_cpu_handle(0), clearColor, 0, nullptr);
		cl->get()->ClearRenderTargetView(m_rtv_descriptor_heap.get_cpu_handle(1), clearColor, 0, nullptr);
		cl->get()->ClearRenderTargetView(m_rtv_descriptor_heap.get_cpu_handle(2), clearColor2, 0, nullptr);
		cl->get()->ClearRenderTargetView(m_rtv_descriptor_heap.get_cpu_handle(3), clearColor2, 0, nullptr);
		cl->get()->ClearRenderTargetView(m_rtv_descriptor_heap.get_cpu_handle(4), clearColor2, 0, nullptr);
		cl->get()->ClearRenderTargetView(m_rtv_descriptor_heap.get_cpu_handle(5), clearColor3, 0, nullptr);
	}

	void perapolygon::draw(command_list* cl)
	{
		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer.get_view());
		cl->get()->SetDescriptorHeaps(1, &m_cbv_srv_usv_descriptor_heap.get());
		cl->get()->SetGraphicsRootDescriptorTable(0, m_cbv_srv_usv_descriptor_heap.get_gpu_handle());
		cl->get()->DrawInstanced(4, 1, 0, 0);

	}

	void perapolygon::draw_shrink_texture_for_blur(command_list* cl)
	{
		cl->get()->IASetVertexBuffers(0, 1, &m_vertex_buffer.get_view());

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle[] = {
			m_rtv_descriptor_heap.get_cpu_handle(3),
			m_rtv_descriptor_heap.get_cpu_handle(4)
		};

		cl->get()->OMSetRenderTargets(2, rtvHandle, false, nullptr);

		cl->get()->SetDescriptorHeaps(1, &m_cbv_srv_usv_descriptor_heap.get());
		cl->get()->SetGraphicsRootDescriptorTable(0, m_cbv_srv_usv_descriptor_heap.get_gpu_handle());

		auto desc = m_float4_resource[Float4ResourceIndex::BLOOM].get()->GetDesc();
		D3D12_VIEWPORT vp{};
		D3D12_RECT sr{};

		vp.MaxDepth = 1.0f;
		vp.MinDepth = 0.0f;
		vp.Height = desc.Height / 2.f;
		vp.Width = desc.Width / 2.f;
		sr.top = 0;
		sr.left = 0;
		sr.right = static_cast<LONG>(vp.Width);
		sr.bottom = static_cast<LONG>(vp.Height);

		for (int i = 0; i < 8; i++)
		{
			cl->get()->RSSetViewports(1, &vp);
			cl->get()->RSSetScissorRects(1, &sr);
			cl->get()->DrawInstanced(4, 1, 0, 0);

			sr.top += static_cast<LONG>(vp.Height);
			vp.TopLeftX = 0;
			vp.TopLeftY = static_cast<FLOAT>(sr.top);

			vp.Width /= 2;
			vp.Height /= 2;
			sr.bottom = static_cast<LONG>(sr.top) + static_cast<LONG>(vp.Height);
		}
	}


}