#pragma once
#include"resource.hpp"
#include<utility>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include<iostream>


namespace ichi
{
	class resource;
	class device;
	class command_list;

	
	//リソースを定数バッファとして作製
	resource* create_constant_resource(device*, unsigned int size);

	//テクスチャのシェーダリソースの取得
	resource* create_texture_resource(device*, command_list*, const DirectX::TexMetadata*, const DirectX::ScratchImage*);

	//インデックスバッファとそのビューの作製
	//std::pair<resource*, D3D12_INDEX_BUFFER_VIEW> create_index_resource_and_view(device*, unsigned int size);

	//頂点バッファとそのビューの作製
	//std::pair<resource*, D3D12_VERTEX_BUFFER_VIEW> create_vertex_resource_and_view(device*, unsigned int size, unsigned int stride);

	//レンダーターゲット用のリソースの作製
	//resource* create_rtv_resource(device*, D3D12_CLEAR_VALUE);

	//リソースへのマップ
	//行列用
	bool map_to_resource(resource*, const DirectX::XMMATRIX&);

	//テクスチャ用
	bool map_to_resource(resource*, const DirectX::Image&);


	//Tはコンテナ、または配列用
	template<typename T>
	bool map_to_resource(resource* resource, T&& container)
	{
		using value_type = std::remove_reference_t<decltype(*std::begin(container))>;

		value_type* target = nullptr;
		auto result = resource->get()->Map(0, nullptr, (void**)&target);
		//失敗したとき
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(container), std::end(container), target);
		resource->get()->Unmap(0, nullptr);

		return true;
	}






}