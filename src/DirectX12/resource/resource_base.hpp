#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;
	class command_list;
}

using namespace DX12;

namespace DX12
{
	


	//リソースのベースのクラス
	class resource_base
	{
		//本体
		ID3D12Resource* m_resource = nullptr;

		//リソースバリアのとき使用
		D3D12_RESOURCE_STATES m_state{};

		//いらなかったら削除するや
		std::optional<D3D12_CLEAR_VALUE> m_clear_value{};

	public:
		resource_base() = default;
		virtual ~resource_base();
		//コピー禁止
		resource_base(const resource_base&) = delete;
		resource_base& operator=(const resource_base&) = delete;
		//ムーブ可能
		resource_base(resource_base&&) noexcept;
		resource_base& operator=(resource_base&&) noexcept;

		//初期化
		bool initialize(device* device, const D3D12_HEAP_PROPERTIES* heapProp, D3D12_HEAP_FLAGS flag,
			const D3D12_RESOURCE_DESC* resoDesc, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue);

		//今のところバックバッファの時とテクスチャの時
		bool initialize(ID3D12Resource* resource);

		//リソースバリア
		void barrior(command_list*, D3D12_RESOURCE_STATES);

		ID3D12Resource* get() noexcept;

		D3D12_CLEAR_VALUE* get_clear_value();

		//有効なポインタを保持しているかどうか
		bool is_empty() const noexcept;
	};


}