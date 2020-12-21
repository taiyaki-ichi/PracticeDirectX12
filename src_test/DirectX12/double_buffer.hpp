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
		IDXGISwapChain4* m_swap_chain = nullptr;
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		std::array<ID3D12Resource*, 2> m_buffer = { nullptr,nullptr };

	public:
		double_buffer() = default;
		~double_buffer();

		//device内で使用
		bool initialize(device*);

		//とりあえずはバックバッファがレンダーターゲットになるまでバリア
		//そのほかの状態までバリアするかは検討す
		void resource_barrior(command_list*);

		//バッファをフリップ
		bool flip();

	};
}