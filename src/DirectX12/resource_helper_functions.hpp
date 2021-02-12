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

	//フォーマットなどを指定することでrtv用のリソースやぺらポリゴンのリソースなどに使用できる
	//命名とかとりあえずって感じ
	resource* create_simple_resource(device*, unsigned int width, unsigned int height, 
		DXGI_FORMAT format,D3D12_RESOURCE_FLAGS flag,D3D12_RESOURCE_STATES state, D3D12_CLEAR_VALUE clearValue);

	//深度バッファの生成
	resource* crate_depth_resource(device*, unsigned int width, unsigned int height);
	

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