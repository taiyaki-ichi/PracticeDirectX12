#pragma once
#include"resource.hpp"
#include<memory>
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
	template<typename>
	class descriptor_heap;
	namespace descriptor_heap_type {
		struct RTV;
	}

	//ダブルバッファ用
	//リソースバリアとかしたり
	class double_buffer
	{
		IDXGIFactory5* m_factory = nullptr;
		IDXGISwapChain4* m_swap_chain = nullptr;
		std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>> m_descriptor_heap{};

		std::array<resource, 2> m_buffer{};
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
		void begin_drawing_to_backbuffer(command_list*, const D3D12_CPU_DESCRIPTOR_HANDLE&);

		//バックバッファへの描写を終了する
		//描写が完了するまでバリアする
		void end_drawing_to_backbuffer(command_list*);

		//バックバッファのクリア
		void clear_back_buffer(command_list*);

		//バッファをフリップ
		void flip();
	};
}