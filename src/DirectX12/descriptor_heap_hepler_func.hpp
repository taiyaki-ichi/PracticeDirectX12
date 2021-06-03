#pragma once
#include"device.hpp"
#include"descriptor_heap_type.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{


	//ディスクリプタヒープにビューを作る関数
	//特殊化できない場合失敗
	template<typename DescriptorHeapType, typename CreateType>
	inline bool create_view_func(device*, ID3D12Resource*, const D3D12_CPU_DESCRIPTOR_HANDLE&);

	//ディスクリプタヒープのタイプがCBV_SRV_UAVの時にCBVのビューを作る
	template<>
	inline bool create_view_func<descriptor_heap_type::CBV_SRV_UAV, view_type::constant_buffer>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		//定数バッファ
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
		cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(resource->GetDesc().Width);

		//定数バッファビューの作成
		device->get()->CreateConstantBufferView(&cbvDesc, cpuHandle);

		return true;
	}


	//ディスクリプタヒープのタイプがCBV_SRV_UAVの時にSRVのビューを作る
	template<>
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, view_type::float4_shader_resource>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		//テクスチャ
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = resource->GetDesc().Format;//RGBA(0.0f～1.0fに正規化)
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

		device->get()->CreateShaderResourceView(resource, //ビューと関連付けるバッファ
			&srvDesc, //先ほど設定したテクスチャ設定情報
			cpuHandle//ヒープのどこに割り当てるか
		);

		return true;
	}


	//ディスクリプタヒープのタイプがCBV_SRV_UAVの時に深度バッファをシェーダリソースとして
	//扱うためのViewの作製
	template<>
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, view_type::depth_stencil_buffer>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC resDesc{};
		resDesc.Format = DXGI_FORMAT_R32_FLOAT;
		resDesc.Texture2D.MipLevels = 1;
		resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		device->get()->CreateShaderResourceView(resource, //ビューと関連付けるバッファ
			&resDesc, //先ほど設定したテクスチャ設定情報
			cpuHandle//ヒープのどこに割り当てるか
		);

		return true;
	}

	//深度バッファを深度バッファとして使うため
	//深度バッファ用のディスクリプタヒープにViewを作る用
	template<>
	inline bool  create_view_func<descriptor_heap_type::DSV, view_type::depth_stencil_buffer>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//フラグは特になし

		device->get()->CreateDepthStencilView(resource, //ビューと関連付けるバッファ
			&dsvDesc, //先ほど設定したテクスチャ設定情報
			cpuHandle//ヒープのどこに割り当てるか
		);

		return true;
	}


	//レンダーターゲット用のディスクリプタヒープに
	//レンダーターゲットのViewを作る
	template<>
	inline bool create_view_func<descriptor_heap_type::RTV, view_type::float4_shader_resource>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		//SRGBレンダーターゲットビュー設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->get()->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

		return true;
	}


	template<>
	inline bool create_view_func<descriptor_heap_type::RTV, view_type::float_shader_resource>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		//SRGBレンダーターゲットビュー設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->get()->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

		return true;
	}

	template<>
	inline bool create_view_func<descriptor_heap_type::CBV_SRV_UAV, view_type::float_shader_resource>
		(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

		device->get()->CreateShaderResourceView(resource, //ビューと関連付けるバッファ
			&srvDesc, //先ほど設定したテクスチャ設定情報
			cpuHandle//ヒープのどこに割り当てるか
		);

		return true;
	}




}