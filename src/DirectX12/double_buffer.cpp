#include"double_buffer.hpp"
#include"command_list.hpp"
#include"device.hpp"
#include"descriptor_heap.hpp"
#include"resource_type_tag.hpp"

#include<iostream>

namespace DX12
{
	double_buffer::~double_buffer()
	{
		if (m_factory)
			m_factory->Release();
		if (m_swap_chain)
			m_swap_chain->Release();
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

			ID3D12Resource* resourcePtr;

			//失敗したとき
			if (FAILED(m_swap_chain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&resourcePtr)))) {
				std::cout << "GetBuffer is failed\n";
				return false;
			}
			
			m_buffer[i].initialize(resourcePtr);
			m_descriptor_heap->create_view<resource_type::RTV>(device, m_buffer[i].get());
		}

		return true;
	}


	D3D12_CPU_DESCRIPTOR_HANDLE double_buffer::get_backbuffer_cpu_handle()
	{
		auto bbIdx = m_swap_chain->GetCurrentBackBufferIndex();
		return m_descriptor_heap->get_cpu_handle(bbIdx);
	}

	void double_buffer::barrior_to_backbuffer(command_list* cl, D3D12_RESOURCE_STATES state)
	{
		auto bbIdx = m_swap_chain->GetCurrentBackBufferIndex();
		m_buffer[bbIdx].barrior(cl, state);
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