#pragma once
#include"../Device.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{

	//DescriptorHeapのテンプレート引数用の型
	//そのディスクリプターヒープの用途を指定
	namespace DescriptorHeapTypeTag
	{
		//定数バッファやシェーダーリソースなど用
		struct CBV_SRV_UAV;
		//ディプスステンシル用
		struct DSV;
		//レンダーターゲット用
		struct RTV;

	}

		//
		//
		//


	namespace DescriptorHeapTypeTag
	{

		struct CBV_SRV_UAV
		{
			//とりあえずすべてのシェーダから見えるようにしておく
			//後でカスタムできるようになるかも
			static ID3D12DescriptorHeap* Initialize(Device* device, int size)
			{
				ID3D12DescriptorHeap* result = nullptr;

				D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
				descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				descHeapDesc.NodeMask = 0;
				descHeapDesc.NumDescriptors = size;
				descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

				if (FAILED(device->Get()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&result))))
					return nullptr;
				else
					return result;
			}

			static unsigned int GetIncrementSize(Device* device)
			{
				return device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
		};


		struct DSV
		{
			static ID3D12DescriptorHeap* Initialize(Device* device, int size)
			{
				ID3D12DescriptorHeap* result = nullptr;

				D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
				dsvHeapDesc.NodeMask = 0;
				dsvHeapDesc.NumDescriptors = size;
				dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
				if (FAILED(device->Get()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&result))))
					return nullptr;
				else
					return result;
			}

			static unsigned int GetIncrementSize(Device* device)
			{
				return device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			}
		};


		struct RTV
		{
			static ID3D12DescriptorHeap* Initialize(Device* device, int size)
			{
				ID3D12DescriptorHeap* result = nullptr;

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
				heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビューなのでRTV
				heapDesc.NodeMask = 0;
				heapDesc.NumDescriptors = size;
				heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//特に指定なし
				if (FAILED(device->Get()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&result))))
					return nullptr;
				else
					return result;
			}

			static unsigned int GetIncrementSize(Device* device)
			{
				return device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			}

		};

	}


}
