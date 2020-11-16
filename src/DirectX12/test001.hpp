#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<iostream>
#include"descriptor_heap.hpp"
#include<array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace graphics::test001
{
	
	//ディスクリプタの構造の通知用の構造体取得
	//テクスチャ、行列定数、深度ステンシルバッファの順
	//ルートシグネチャにぶち込む
	std::array<D3D12_DESCRIPTOR_RANGE, 3> get_descriptor_ranges()
	{
		//テクスチャ
		D3D12_DESCRIPTOR_RANGE textureDesc{};
		textureDesc.NumDescriptors = 1;
		textureDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		//SRVの0番目
		textureDesc.BaseShaderRegister = 0;
		textureDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//座標の行列
		D3D12_DESCRIPTOR_RANGE matDesc{};
		matDesc.NumDescriptors = 1;
		matDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		//CBV(定数バッファ)の0番目
		matDesc.BaseShaderRegister = 0;
		matDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//深度ステンシルバッファ
		D3D12_DESCRIPTOR_RANGE stencilDesc{};
		stencilDesc.NumDescriptors = 1;
		stencilDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		//SRVの0番目
		stencilDesc.BaseShaderRegister = 0;
		stencilDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

}