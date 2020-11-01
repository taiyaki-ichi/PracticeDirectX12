#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<memory>
#include<array>
#include<vector>
#include<iostream>
#include<wrl.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace graphics
{

	//namespace {

		using namespace Microsoft::WRL;

		//�f�o�b�N�̂Ƃ��p
		bool init_debug() {
			ID3D12Debug* debugLayer = nullptr;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
				debugLayer->EnableDebugLayer();
				debugLayer->Release();
				return true;
			}
			else {
				std::cout << __func__ << " is failed\n";
				return false;
			}
		}

		//�t�@�N�g���̐���
		ComPtr<IDXGIFactory6> create_dxgi_factory()
		{
			//�Ƃ肠����6�ł��
			ComPtr<IDXGIFactory6> factory = nullptr;

			//�Ƃ肠����1�̂ق����g���Ă���Ă݂�
			if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
				std::cout << __func__ << " is failed\n";
				return nullptr;
			}
			
			return factory;

			//2�̕��A�\��
			/*
			if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)))) {
				if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
					return nullptr;
				}
			}
			*/
		}

		//�K�؂ȃA�_�v�^�[�̎擾
		ComPtr<IDXGIAdapter1> get_adapter(const ComPtr<IDXGIFactory6>& factory)
		{
			ComPtr<IDXGIAdapter1> adapter = nullptr;

			for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				//�\���o�͂̂Ȃ������_�����O�p�̃f�o�C�X�̏ꍇ�����B�p�����Ăق���T��
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;
				//�K�؂ȃA�_�v�^�����������ꍇ
				else
					return adapter;
			}

			//������Ȃ������ꍇ
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}

		//�f�o�C�X�̎擾
		ComPtr<ID3D12Device> create_device(const ComPtr<IDXGIAdapter1>& adapter)
		{
			ComPtr<ID3D12Device> device = nullptr;

			//�t�B�[�`�����x����񋓂��Ă���
			std::array levels{
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0
			};

			for (auto l : levels) {
				//�K�؂ɐ����ł����ꍇ
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), l, IID_PPV_ARGS(&device))))
					return device;
			}

			//���s�����Ƃ�
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}

		//�R�}���h�A���P�[�^�̍쐻
		ComPtr<ID3D12CommandAllocator> create_command_allocator(const ComPtr<ID3D12Device>& device) {
			ComPtr<ID3D12CommandAllocator> allocator = nullptr;
			//�����̏ꍇ
			if (SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
				return allocator;
			//���s�����ꍇ
			else {
				std::cout << __func__ << " is failed\n";
				return nullptr;
			}
		}

		//�R�}���h���X�g�̍쐻
		ComPtr<ID3D12GraphicsCommandList> create_command_list(const ComPtr<ID3D12Device>& device,const ComPtr<ID3D12CommandAllocator>& allocator) {
			ComPtr<ID3D12GraphicsCommandList> list = nullptr;
			//����
			if (SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&list))))
				return list;
			//���s
			else {
				std::cout << __func__ << " is failed\n";
				return nullptr;
			}
		}

		//�R�}���h�L���[�̐����B�^�C�v�����킹�邽��list���Q��
		ComPtr<ID3D12CommandQueue> create_command_queue(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& list)
		{
			D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
			cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//�^�C���A�E�g�i�V
			cmdQueueDesc.NodeMask = 0;
			cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//�v���C�I���e�B���Ɏw��Ȃ�
			cmdQueueDesc.Type = list.Get()->GetType();			//�����̓R�}���h���X�g�ƍ��킹��

			ComPtr<ID3D12CommandQueue> queue = nullptr;
			if(SUCCEEDED(device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&queue))))
				return queue;
			else {
				std::cout << __func__ << " is failed\n";
				return nullptr;
			}
		}

		//�X���b�v�`�F�C���̍쐻�A1�ł���Ă݂�
		ComPtr<IDXGISwapChain4> create_swap_chain(const ComPtr<IDXGIFactory6> factory, HWND hwnd,const ComPtr<ID3D12CommandQueue>& queue)
		{
			IDXGISwapChain4* swapChainPtr = nullptr;

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

			if (SUCCEEDED(factory->CreateSwapChainForHwnd(queue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChainPtr)))
				return ComPtr<IDXGISwapChain4>{swapChainPtr};
			else {
				std::cout << __func__ << " is failed\n";
				return nullptr;
			}
		}

		//�ŃX�N���v�^�q�[�v�̐���
		ComPtr<ID3D12DescriptorHeap> create_descriptor_heap(const ComPtr<ID3D12Device>& device)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//�����_�[�^�[�Q�b�g�r���[�Ȃ̂œ��RRTV
			heapDesc.NodeMask = 0;
			heapDesc.NumDescriptors = 2;//�\���̂Q��
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//���Ɏw��Ȃ�

			
			ID3D12DescriptorHeap* descriptorHeap = nullptr;
			if (SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap))))
				return ComPtr<ID3D12DescriptorHeap>{descriptorHeap};
			else {
				std::cout << __func__ << " is failed\n";
				return nullptr;
			}

			return nullptr;
		}

		//�o�b�t�@�̐ݒ�A�߂�l�͂ǂ����ɑ������Ă����ׂ�����
		std::vector<ComPtr<ID3D12Resource>> create_buffers(const ComPtr<ID3D12Device>& device,const ComPtr<IDXGISwapChain1>& sc,const ComPtr<ID3D12DescriptorHeap>& dh)
		{
			DXGI_SWAP_CHAIN_DESC swcDesc{};
			sc->GetDesc(&swcDesc);
			std::vector< ComPtr<ID3D12Resource>> backBuffers(swcDesc.BufferCount);
			D3D12_CPU_DESCRIPTOR_HANDLE handle = dh->GetCPUDescriptorHandleForHeapStart();
			for (size_t i = 0; i < swcDesc.BufferCount; ++i) {
				//���s�����ꍇ
				if (FAILED(sc->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&backBuffers[i])))) {
					backBuffers.clear();
					std::cout << __func__ << " is failed\n";
					return backBuffers;
				}
					
				device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
				handle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			}
			return backBuffers;
		}

		//�t�F���X�̐���
		std::pair<ComPtr<ID3D12Fence>,UINT64> create_fence(const ComPtr<ID3D12Device>& device)
		{
			ComPtr<ID3D12Fence> fence = nullptr;
			UINT64 val = 0;
			if (SUCCEEDED(device->CreateFence(val, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
				return { fence,val };
			else {
				std::cout << __func__ << " is failed\n";
				return { nullptr,0 };
			}
		}

	//}


}