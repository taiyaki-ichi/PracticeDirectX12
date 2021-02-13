#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;

	class command_list
	{
		ID3D12CommandAllocator* m_allocator = nullptr;
		ID3D12CommandQueue* m_queue = nullptr;

		ID3D12GraphicsCommandList* m_list = nullptr;

		//仮
		ID3D12Fence* m_fence = nullptr;
		HANDLE m_fence_event = nullptr;
		UINT64 m_fence_value = 1;

	public:
		command_list() = default;
		~command_list();

		bool initialize(device*);

		ID3D12GraphicsCommandList* get();
		ID3D12CommandQueue* get_queue();
		ID3D12CommandAllocator* get_allocator();

		//コマンドの実行
		void execute();
		
		//コマンドのクリア
		//引数は初期設定したいパイプラインステート
		void clear();

		//レンダーターゲットの設定
		//どのターゲットのViewもおなじディスクリプタヒープ連続して生成されていないとみなしている
		void set_render_target(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle);
		void set_render_target(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle);
		void set_render_target(unsigned int renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget);
		void set_render_target(unsigned int renderTagetHandleNum, D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle);

		//ビューポートの設定
		void set_viewport(const D3D12_VIEWPORT& viewport);
		void set_viewport(unsigned int num, D3D12_VIEWPORT* viewportPtr);

		//シザー矩形の設定
		void set_scissor_rect(const D3D12_RECT& rect);
		void set_scissor_rect(unsigned int num, D3D12_RECT* rectPtr);

		//Closeを呼び出す
		void close();
	};

}