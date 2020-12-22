#pragma once
#include<array>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;

	//ダブルバッファ用
	//リソースバリアとかしたり
	class double_buffer
	{
		IDXGIFactory6* m_factory = nullptr;
		IDXGISwapChain4* m_swap_chain = nullptr;
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		std::array<ID3D12Resource*, 2> m_buffer = { nullptr,nullptr };

	public:
		double_buffer() = default;
		~double_buffer();

		//device内で使用
		//hwndは仮
		//queueはdeviceに持たせるか？？
		bool initialize(device*,HWND,command_list*);

		//描写開始時のリソースバリア
		//バックバッファが描写ターゲットになるまで待つ
		void begin_resource_barrior(command_list*);
		//描写終了時のリソースバリア
		//バックバッファへの描写完了を待つ
		void end_resource_barrior(command_list*);

		//連だーターゲットの取得
		D3D12_CPU_DESCRIPTOR_HANDLE get_render_target(device*);

		//バッファをフリップ
		void flip();
	};
}