#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<iostream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace graphics
{
	//ディスクリプタヒープの生成
	ID3D12DescriptorHeap* create_texture_descriptor_heap(ID3D12Device* device)
	{
		ID3D12DescriptorHeap* descHeap = nullptr;
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
		descHeapDesc.NodeMask = 0;//マスクは0
		descHeapDesc.NumDescriptors = 1;//ビューは今のところ１つだけ
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//シェーダリソースビュー(および定数、UAVも)

		if (FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap)))) {
			std::cout << __func__ << " is falied\n";
			return nullptr;
		}
		return descHeap;
	}

	//テクスチャ用の設定
	void set_texture_view(ID3D12Device* device, ID3D12DescriptorHeap* dscHeap, ID3D12Resource* textureBuffer)
	{
		auto desc = textureBuffer->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;//DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA(0.0f～1.0fに正規化)
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//後述
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

		device->CreateShaderResourceView(textureBuffer, //ビューと関連付けるバッファ
			&srvDesc, //先ほど設定したテクスチャ設定情報
			dscHeap->GetCPUDescriptorHandleForHeapStart()//ヒープのどこに割り当てるか
		);
	}



}