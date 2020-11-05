#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<array>
#include<vector>
#include<iostream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace graphics
{



	//デバックのとき用
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

	//ファクトリの生成
	IDXGIFactory6* create_dxgi_factory()
	{
		//とりあえず6でやる
		IDXGIFactory6* factory = nullptr;

		//とりあえず1のほうを使ってやってみる
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}

		return factory;

		//2の方、予備
		/*
		if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)))) {
			if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
				return nullptr;
			}
		}
		*/
	}

	//適切なアダプターの取得
	IDXGIAdapter1* get_adapter(IDXGIFactory6* factory)
	{
		IDXGIAdapter1* adapter = nullptr;

		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			//表示出力のないレンダリング用のデバイスの場合無視。継続してほかを探す
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;
			//適切なアダプタが見つかった場合
			else
				return adapter;
		}

		//見つからなかった場合
		std::cout << __func__ << " is failed\n";
		return nullptr;
	}

	//デバイスの取得
	ID3D12Device* create_device(IDXGIAdapter1* adapter)
	{
		ID3D12Device* device = nullptr;

		//フィーチャレベルを列挙しておく
		std::array levels{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		for (auto l : levels) {
			//適切に生成できた場合
			if (SUCCEEDED(D3D12CreateDevice(adapter, l, IID_PPV_ARGS(&device))))
				return device;
		}

		//失敗したとき
		std::cout << __func__ << " is failed\n";
		return nullptr;
	}

	//コマンドアロケータの作製
	ID3D12CommandAllocator* create_command_allocator(ID3D12Device* device) {
		ID3D12CommandAllocator* allocator = nullptr;
		//成功の場合
		if (SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
			return allocator;
		//失敗した場合
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}

	//コマンドリストの作製
	ID3D12GraphicsCommandList* create_command_list(ID3D12Device* device, ID3D12CommandAllocator* allocator) {
		ID3D12GraphicsCommandList* list = nullptr;
		//成功
		if (SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&list))))
			return list;
		//失敗
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}

	//コマンドキューの生成。タイプを合わせるためlistも参照
	ID3D12CommandQueue* create_command_queue(ID3D12Device* device, ID3D12GraphicsCommandList* list)
	{
		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
		cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//タイムアウトナシ
		cmdQueueDesc.NodeMask = 0;
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//プライオリティ特に指定なし
		cmdQueueDesc.Type = list->GetType();			//ここはコマンドリストと合わせる

		ID3D12CommandQueue* queue = nullptr;
		if (SUCCEEDED(device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&queue))))
			return queue;
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}

	//スワップチェインの作製、1でやってみる
	IDXGISwapChain4* create_swap_chain(IDXGIFactory6* factory, ID3D12CommandQueue* queue, HWND hwnd)
	{
		IDXGISwapChain4* swapChainPtr = nullptr;

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

		if (SUCCEEDED(factory->CreateSwapChainForHwnd(queue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChainPtr)))
			return swapChainPtr;
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}

	//でスクリプタヒープの生成
	ID3D12DescriptorHeap* create_descriptor_heap(ID3D12Device* device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビューなので当然RTV
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = 2;//表裏の２つ
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//特に指定なし


		ID3D12DescriptorHeap* descriptorHeap = nullptr;
		if (SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap))))
			return descriptorHeap;
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}

		return nullptr;
	}

	//バッファの設定、戻り値はどっかに束縛しておくべきかも
	std::vector<ID3D12Resource*> create_double_buffer(ID3D12Device* device, IDXGISwapChain1* sc, ID3D12DescriptorHeap* dh)
	{
		DXGI_SWAP_CHAIN_DESC swcDesc{};
		sc->GetDesc(&swcDesc);
		std::vector<ID3D12Resource*> backBuffers(swcDesc.BufferCount);
		D3D12_CPU_DESCRIPTOR_HANDLE handle = dh->GetCPUDescriptorHandleForHeapStart();

		//SRGBレンダーターゲットビュー設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		for (size_t i = 0; i < swcDesc.BufferCount; ++i) {
			//失敗した場合
			if (FAILED(sc->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&backBuffers[i])))) {
				backBuffers.clear();
				std::cout << __func__ << " is failed\n";
				return backBuffers;
			}

			device->CreateRenderTargetView(backBuffers[i], &rtvDesc, handle);
			handle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}
		return backBuffers;
	}

	//フェンスの生成
	std::pair<ID3D12Fence*, UINT64> create_fence(ID3D12Device* device)
	{
		ID3D12Fence* fence = nullptr;
		UINT64 val = 0;
		if (SUCCEEDED(device->CreateFence(val, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
			return { fence,val };
		else {
			std::cout << __func__ << " is failed\n";
			return { nullptr,0 };
		}
	}

}