#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

//ディスクリプタヒープの型関連

namespace DX12
{

	//view生成用
	//生成方法の静的な関数が定義されている
	//あと、有効なディスクリプタヒープのタイプかどうかの判定も
	namespace view_type {

		//定数バッファ
		struct constant_buffer {};

		//深度ステンシル
		struct depth_stencil_buffer {};

		//RGBAのテクスチャのようなデータ
		struct float4_shader_resource {};

		//Rのみのテクスチャのようなデータ
		struct float_shader_resource {};
	}


	//作製するViewのタイプを指定するトレイツ
	//namespace view_typeの型をusingする
	//descriptor_heap::create_view内で使用
	template<typename T>
	struct DefaultViewTypeTraits {
		using view_type;
	};


	//実際のリソースのポインタを取り出すためのポリシー
	//descriptor_heap::create_viewないで使用
	template<typename T>
	struct GetResourcePtrPolicy {
		static ID3D12Resource* get_resource_ptr(T*) {
			return nullptr;
		}
	};


	//タイプ指定用
	//初期化用の関数の定義
	//インクリメントの幅の取得用関数とか
	//定義はdescriptor_heap.hpp内に
	namespace descriptor_heap_type {
		struct CBV_SRV_UAV;
		struct DSV;
		struct RTV;
	};


	
}