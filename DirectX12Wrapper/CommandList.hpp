#pragma once
#include"Device.hpp"

namespace DX12
{
	
	class CommandList
	{
		ID3D12CommandAllocator* allocator = nullptr;
		ID3D12CommandQueue* queue = nullptr;

		ID3D12GraphicsCommandList* list = nullptr;

		ID3D12Fence* fence = nullptr;
		std::uint64_t fenceValue = 1;

	public:
		CommandList() = default;
		~CommandList();

		CommandList(const  CommandList&) = delete;
		CommandList& operator=(const CommandList&) = delete;

		CommandList(CommandList&&) noexcept;
		CommandList& operator=(CommandList&&) noexcept;

		void Initialize(Device*);

		ID3D12GraphicsCommandList* Get();

		std::pair<IDXGIFactory5*, IDXGISwapChain4*> CreateFactryAndSwapChain(HWND);

		//コマンドの実行
		void Execute();

		//コマンドのクリア
		//引数は初期設定したいパイプラインステート
		void Clear();

		//レンダーターゲットの設定
		//OMSetRenderTargetsの最適化について、どのターゲットのViewもおなじディスクリプタヒープ連続して生成されていないとみなしている
		void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle);
		void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle);
		void SetRenderTarget(std::uint32_t renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget);
		void SetRenderTarget(std::uint32_t renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle);

		//ビューポートの設定
		void SetViewport(const D3D12_VIEWPORT& viewport);
		void SetViewport(std::uint32_t num, D3D12_VIEWPORT* viewportPtr);

		//シザー矩形の設定
		void SetScissorRect(const D3D12_RECT& rect);
		void SetScissorRect(std::uint32_t num, D3D12_RECT* rectPtr);

		//Closeを呼び出す
		void Close();
	};


	//
	//
	//

	inline CommandList::~CommandList()
	{
		if (allocator)
			allocator->Release();
		if (queue)
			queue->Release();
		if (list)
			list->Release();
		if (fence)
			fence->Release();
	}

	inline CommandList::CommandList(CommandList&& rhs) noexcept
	{
		allocator = rhs.allocator;
		queue = rhs.queue;
		list = rhs.list;
		fence = rhs.fence;
		fenceValue = rhs.fenceValue;
		rhs.allocator = nullptr;
		rhs.queue = nullptr;
		rhs.list = nullptr;
		rhs.fence = nullptr;
	}

	inline CommandList& CommandList::operator=(CommandList&& rhs) noexcept
	{
		allocator = rhs.allocator;
		queue = rhs.queue;
		list = rhs.list;
		fence = rhs.fence;
		fenceValue = rhs.fenceValue;
		rhs.allocator = nullptr;
		rhs.queue = nullptr;
		rhs.list = nullptr;
		rhs.fence = nullptr;
		return *this;
	}

	inline void CommandList::Initialize(Device* device)
	{
		if (FAILED(device->Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
			throw "CreateCommandAllocator is failed\n";

		if (FAILED(device->Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&list))))
			throw "CreateCommandList is failed\n";

		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
		cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//タイムアウトナシ
		cmdQueueDesc.NodeMask = 0;
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//プライオリティ特に指定なし
		cmdQueueDesc.Type = list->GetType();			//ここはコマンドリストと合わせる
		if (FAILED(device->Get()->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&queue))))
			throw "CreateCommandQueue is failed\n";

		if (FAILED(device->Get()->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
			throw "CreateFence is failed\n";

	}

	inline ID3D12GraphicsCommandList* CommandList::Get()
	{
		return list;
	}

	inline std::pair<IDXGIFactory5*, IDXGISwapChain4*> CommandList::CreateFactryAndSwapChain(HWND hwnd)
	{
		IDXGIFactory5* factory = nullptr;
		IDXGISwapChain4* swapChain = nullptr;

		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
			throw "CreateDXGIFactory1 is failed\n";

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

		if (FAILED(factory->CreateSwapChainForHwnd(queue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain)))
			throw "CreateSwapChainForHwnd is failed\n";

		return std::make_pair(factory, swapChain);
	}

	inline void CommandList::Execute()
	{
		queue->ExecuteCommandLists(1, (ID3D12CommandList**)(&list));
		queue->Signal(fence, ++fenceValue);

		//コマンドキューが終了していないことの確認
		if (fence->GetCompletedValue() < fenceValue)
		{
			HANDLE fenceEvent = nullptr;
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			// イベントが発火するまで待つ
			WaitForSingleObject(fenceEvent, INFINITE);
		}
	}

	inline void CommandList::Clear()
	{
		allocator->Reset();
		list->Reset(allocator, nullptr);
	}

	inline void CommandList::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle)
	{
		list->OMSetRenderTargets(1, &renderTargetHandle, false, nullptr);
	}

	inline void CommandList::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle)
	{
		list->OMSetRenderTargets(1, &renderTargetHandle, false, &depthStencilHandle);
	}

	inline void CommandList::SetRenderTarget(std::uint32_t renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget)
	{
		list->OMSetRenderTargets(renderTagetHandleNum, renderTarget, false, nullptr);
	}

	inline void CommandList::SetRenderTarget(std::uint32_t renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle)
	{
		list->OMSetRenderTargets(renderTagetHandleNum, renderTarget, false, &depthStencilHandle);
	}

	inline void CommandList::SetViewport(const D3D12_VIEWPORT& viewport)
	{
		list->RSSetViewports(1, &viewport);
	}

	inline void CommandList::SetViewport(std::uint32_t num, D3D12_VIEWPORT* viewportPtr)
	{
		list->RSSetViewports(num, viewportPtr);
	}

	inline void CommandList::SetScissorRect(const D3D12_RECT& rect)
	{
		list->RSSetScissorRects(1, &rect);
	}

	inline void CommandList::SetScissorRect(std::uint32_t num, D3D12_RECT* rectPtr)
	{
		list->RSSetScissorRects(num, rectPtr);
	}

	inline void CommandList::Close()
	{
		list->Close();
	}


}