#include"double_buffer.hpp"
#include"command_list.hpp"
#include"device.hpp"
#include"descriptor_heap.hpp"
#include"resource_type_tag.hpp"

#include<iostream>

namespace ichi
{
	double_buffer::~double_buffer()
	{
		if (m_factory)
			m_factory->Release();
		if (m_swap_chain)
			m_swap_chain->Release();
		for (size_t i = 0; i < m_buffer.size(); i++)
			if (m_buffer[i])
				m_buffer[i]->Release();
	}

	bool double_buffer::initialize(device* device,HWND hwnd,command_list* cl)
	{
		//ファクトリの作製
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)))) {
			std::cout << "CreateDXGIFactory1 is failed\n";
			return false;
		}

		//スワップチェイン作成

		//ウィンドウの大きさ取得用
		RECT windowRect{};
		GetWindowRect(hwnd, &windowRect);

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
		swapchainDesc.Width = windowRect.right - windowRect.left;
		swapchainDesc.Height = windowRect.bottom - windowRect.top;
		swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.Stereo = false;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
		swapchainDesc.BufferCount = 2;
		swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		if (FAILED(m_factory->CreateSwapChainForHwnd(cl->get_queue(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&m_swap_chain))) {
			std::cout << "CreateSwapChainForHwnd is failed\n";
			return false;
		}
			
		//ディスクリプタヒープ
		m_descriptor_heap = std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>>{
			device->create<descriptor_heap<descriptor_heap_type::RTV>>(2)
		};
		if (!m_descriptor_heap) {
			std::cout << "double buufer DescriptorHeap is failed\n";
			return false;
		}


		for (size_t i = 0; i < m_buffer.size(); i++) {

			//失敗したとき
			if (FAILED(m_swap_chain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&m_buffer[i])))) {
				std::cout << "GetBuffer is failed\n";
				return false;
			}
			m_descriptor_heap->create_view<create_view_type::RTV>(device, m_buffer[i]);
		}

		return true;
	}

	void double_buffer::begin_drawing_to_backbuffer(command_list* cl, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
	{
		auto bbIdx = m_swap_chain->GetCurrentBackBufferIndex();

		//リソースバリアの作製
		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_buffer[bbIdx];
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		cl->get()->ResourceBarrier(1, &BarrierDesc);


		//レンダーターゲットの作製
		auto rtvH = m_descriptor_heap->get_cpu_handle(bbIdx);
		cl->get()->OMSetRenderTargets(1, &rtvH, false, &handle);
	}

	void double_buffer::end_drawing_to_backbuffer(command_list* cl)
	{
		auto bbIdx = m_swap_chain->GetCurrentBackBufferIndex();

		//リソースバリアの作製
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_buffer[bbIdx];
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

	
	}

	void double_buffer::clear_back_buffer(command_list* cl)
	{
		auto bbIdx = m_swap_chain->GetCurrentBackBufferIndex();
		auto rtvH = m_descriptor_heap->get_cpu_handle(bbIdx);
		//バックバッファのクリア
		float clearColor[] = { 0.5f,0.5f,0.5f,1.0f };
		cl->get()->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		
	}

	void double_buffer::flip()
	{
		m_swap_chain->Present(1, 0);
	}

}