#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;


	//仮
	constexpr unsigned int DESCRIPTOR_HEAP_SIZE = 128;

	//ConstantBuffer用とShaderResouce用の2つ作る
	//sapmerはstaticにRootSignatureで設定
	template<typename Value>
	class descriptor_heap
	{
		ID3D12Resource* m_resource = nullptr;

		unsigned int m_offset = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap();

		bool initialize(unsigned int size);

		//コンテナを引数にディスクリプタヒープ内にビューを作製
		//コンテナのサイズはDESCRIPTOR_HEAP_SIZE以下
		//描写毎に呼び出してビューを作製
		//テクスチャか定数バッファかで処理が変わる
		//一つずつ受け入れるようにするか
		void create_view(Value&&);

		//offsetを0にする
		void reset();
	};

}