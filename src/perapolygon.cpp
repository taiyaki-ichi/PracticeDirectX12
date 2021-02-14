#include"perapolygon.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/resource_type_tag.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/resource_helper_functions.hpp"
#include"window_size.hpp"
#include"scene_data.hpp"

namespace DX12
{


	bool perapolygon::initialize(device* device, ID3D12Resource* depthResource)
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
		m_resource[ResourceIndex::COLOR] = create_simple_resource(device, window_width, window_height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, { DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.5f,0.5f,0.5f,1.f } });

		m_resource[ResourceIndex::NORMAL] = create_simple_resource(device, window_width, window_height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, { DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.5f,0.5f,0.5f,1.f } });

		m_resource[ResourceIndex::BLOOM] = create_simple_resource(device, window_width, window_height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, { DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.f,0.f,0.f,1.f } });

		m_resource[ResourceIndex::SHRINK_BLOOM] = create_simple_resource(device, window_width / 2, window_height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, { DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.f,0.f,0.f,1.f } });

		m_resource[ResourceIndex::DOF] = create_simple_resource(device, window_width / 2, window_height,
			DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, { DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.f,0.f,0.f,1.f } });

		m_resource[ResourceIndex::SSAO] = create_simple_resource(device, window_width , window_height,
			DXGI_FORMAT_R32_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, { DXGI_FORMAT_R32_FLOAT,{ 1.f,1.f,1.f,1.f } });

		m_resource[ResourceIndex::CBV] = create_constant_resource(device, sizeof(scene_data_for_perapolygon));



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

			map_to_resource(&m_vertex_buffer, pv);
		}

		//レンダーターゲット用のディスクリプタヒープにViewを作製
		for (int i = 0; i < RESOURCE_NUM - 2; i++)
		{
			auto result = m_rtv_descriptor_heap.create_view<resource_type::RTV>(device, m_resource[i].get());
			if (!result) {
				std::cout << "pera rtv create view " << i << " is failed\n";
				return false;
			}
		}
		//深度用としてSSAO用のViewを生成
		{
			auto result = m_rtv_descriptor_heap.create_view<resource_type::R32>(device, m_resource[ResourceIndex::SSAO].get());
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
			auto result = m_cbv_srv_usv_descriptor_heap.create_view<resource_type::SRV>(device, m_resource[i].get());
			if (!result) {
				std::cout << "pera srv create view " << i << " is failed\n";
				return false;
			}
		}
		{
			auto result = m_cbv_srv_usv_descriptor_heap.create_view<resource_type::R32>(device, m_resource[ResourceIndex::SSAO].get());
			if (!result) {
				std::cout << "pera srv create view dsv is failed\n";
				return false;
			}
		}
	
		{
			auto result = m_cbv_srv_usv_descriptor_heap.create_view<resource_type::CBV>(device, m_resource[ResourceIndex::CBV].get());
			if (!result) {
				std::cout << "pera srv create view cbv is failed\n";
				return false;
			}
		}

		//最後に深度バッファのViewを生成
		//つまり添え字はRESOURCE_NUM
		{
			auto result = m_cbv_srv_usv_descriptor_heap.create_view<resource_type::DSV>(device, depthResource);
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
		for (int i = 0; i < RESOURCE_NUM-1; i++)
			m_resource[i].barrior(cl, state);
	}

	void perapolygon::ssao_resource_barrior(command_list* cl, D3D12_RESOURCE_STATES state)
	{
		m_resource[ResourceIndex::SSAO].barrior(cl, state);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE perapolygon::get_ssao_cpu_handle()
	{
		return m_rtv_descriptor_heap.get_cpu_handle(ResourceIndex::SSAO);
	}

	void perapolygon::map_scene_data(const scene_data_for_perapolygon& sceneData)
	{
		scene_data_for_perapolygon* ptr = nullptr;
		m_resource[ResourceIndex::CBV].get()->Map(0, nullptr, (void**)&ptr);

		*ptr = sceneData;

		m_resource[ResourceIndex::CBV].get()->Unmap(0, nullptr);
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

		auto desc = m_resource[ResourceIndex::BLOOM].get()->GetDesc();
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