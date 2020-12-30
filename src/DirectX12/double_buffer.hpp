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
	class depth_buffer;

	//ダブルバッファ用
	//リソースバリアとかしたり
	class double_buffer
	{
		IDXGIFactory6* m_factory = nullptr;
		IDXGISwapChain4* m_swap_chain = nullptr;
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		std::array<ID3D12Resource*, 2> m_buffer = { nullptr,nullptr };

		//バッファをインクリメントするようにメモしておく
		//begin_drawing_to_backbafferでdeviceを参照したくない
		UINT m_descriptor_handle_increment_size = 0;

	public:
		double_buffer() = default;
		~double_buffer();

		//device内で使用
		//hwndは仮
		//queueはdeviceに持たせるか？？
		bool initialize(device*,HWND,command_list*);

		//バックバッファへの描写を開始する
		//バックバッファをリソースバリアし、レンダーターゲットに指定
		//あと、バッファのクリア
		void begin_drawing_to_backbuffer(command_list*,depth_buffer*);
		//バックバッファへの描写を終了する
		//描写が完了するまでバリアする
		void end_drawing_to_backbuffer(command_list*);

		//バックバッファのクリア
		void clear_back_buffer(command_list*);

		//バッファをフリップ
		void flip();
	};
}