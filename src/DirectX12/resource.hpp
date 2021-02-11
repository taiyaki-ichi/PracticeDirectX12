#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class command_list;

	//リソース（バッファ）のクラス
	class resource
	{
		//本体
		ID3D12Resource* m_resource = nullptr;

		//リソースバリアのとき使用
		D3D12_RESOURCE_STATES m_state{};

	public:
		resource() = default;
		~resource();

		//初期化
		bool initialize(device*, const D3D12_HEAP_PROPERTIES*, D3D12_HEAP_FLAGS,
			const D3D12_RESOURCE_DESC*, D3D12_RESOURCE_STATES, const D3D12_CLEAR_VALUE*);

		//リソースバリア
		void barrior(command_list*, D3D12_RESOURCE_STATES);

		ID3D12Resource* get() noexcept;

		//有効なポインタを保持しているかどうか
		bool is_empty() const noexcept;
	};


}