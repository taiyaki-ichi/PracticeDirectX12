#include"double_buffer.hpp"
#include"command_list.hpp"
#include"device.hpp"

#include<iostream>

namespace ichi
{
	double_buffer::~double_buffer()
	{
		if (m_factory)
			m_factory->Release();
		if (m_descriptor_heap)
			m_descriptor_heap->Release();
		if (m_swap_chain)
			m_swap_chain->Release();
		for (int i = 0; i < m_buffer.size(); i++)
			if (m_buffer[i])
				m_buffer[i]->Release();
	}

	bool double_buffer::initialize(device* device,HWND hwnd,command_list* cl)
	{
		//�t�@�N�g���̍쐻
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)))) {
			std::cout << "CreateDXGIFactory1 is failed\n";
			return false;
		}

		//�X���b�v�`�F�C���쐬

		//�E�B���h�E�̑傫���擾�p
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
			
		//�f�B�X�N���v�^�q�[�v
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//�����_�[�^�[�Q�b�g�r���[�Ȃ̂œ��RRTV
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = 2;//�\���̂Q��
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//���Ɏw��Ȃ�
		if (FAILED(device->get()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptor_heap)))) {
			std::cout << "CreateDescriptorHeap is failed\n";
			return false;
		}
			
		//�r���[�쐬
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		//SRGB�����_�[�^�[�Q�b�g�r���[�ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		for (int i = 0; i < m_buffer.size(); i++) {

			//���s�����Ƃ�
			if (FAILED(m_swap_chain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&m_buffer[i])))) {
				std::cout << "GetBuffer is failed\n";
				return false;
			}
			device->get()->CreateRenderTargetView(m_buffer[i], &rtvDesc, handle);
			
			handle.ptr += device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);	
		}

		//�C���N�������g�p�̃T�C�Y�̃���
		m_descriptor_handle_increment_size = device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		return true;
	}

	void double_buffer::begin_drawing_to_backbuffer(command_list* cl)
	{
		auto bbIdx = m_swap_chain->GetCurrentBackBufferIndex();

		//���\�[�X�o���A�̍쐻
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_buffer[bbIdx];
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		//�����_�[�^�[�Q�b�g�̍쐻
		auto rtvH = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * m_descriptor_handle_increment_size);
		cl->get()->OMSetRenderTargets(1, &rtvH, false, nullptr);

		//�o�b�N�o�b�t�@�̃N���A
		//�Ƃ肠�������F��
		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//���F
		cl->get()->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);


	}

	void double_buffer::end_drawing_to_backbuffer(command_list* cl)
	{
		auto bbIdx = m_swap_chain->GetCurrentBackBufferIndex();

		//���\�[�X�o���A�̍쐻
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_buffer[bbIdx];
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		cl->get()->ResourceBarrier(1, &BarrierDesc);
	}

	void double_buffer::flip()
	{
		m_swap_chain->Present(1, 0);
	}

}