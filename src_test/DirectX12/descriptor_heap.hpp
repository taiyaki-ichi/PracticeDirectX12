#pragma once
#include"device.hpp"
#include<type_traits>
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
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		unsigned int m_size = 0;
		unsigned int m_offset = 0;
		unsigned int m_increment_size = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap();

		bool initialize(device* d, unsigned int size);

		//描写毎に呼び出してビューを作製
		//テクスチャか定数バッファかで処理が変わる
		//一つずつ受け入れるようにするか
		bool create_view(device* device, Value* resourcePtr);

		//offsetを0にする
		void reset() noexcept;

		ID3D12DescriptorHeap*& get() noexcept;
	};



	template<typename Value>
	inline descriptor_heap<Value>::~descriptor_heap(){
		if (m_descriptor_heap)
			m_descriptor_heap->Release();
	}

	template<typename Value>
	inline bool descriptor_heap<Value>::initialize(device* device, unsigned int size)
	{

		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
		descHeapDesc.NodeMask = 0;//マスクは0
		descHeapDesc.NumDescriptors = size;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//シェーダリソースビュー(および定数、UAVも)

		if (FAILED(device->get()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_descriptor_heap)))) {
			std::cout << __func__ << " is falied\n";
			return false;
		}

		m_size = size;
		m_increment_size = device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		return true;
	}

	template<typename Value>
	inline bool descriptor_heap<Value>::create_view(device* device, Value* resourcePtr)
	{

		auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		heapHandle.ptr += m_increment_size * m_offset;

		if constexpr (std::is_same_v<Value, constant_buffer_resource>) {
			//定数バッファ
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
			cbvDesc.BufferLocation = resourcePtr->get()->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(resourcePtr->get()->GetDesc().Width);

			//定数バッファビューの作成
			device->get()->CreateConstantBufferView(&cbvDesc, heapHandle);

			m_offset++;

			return true;
		}
		else if constexpr (std::is_same_v<Value, texture_shader_resource>) {
			//テクスチャ
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = resourcePtr->get()->GetDesc().Format;//RGBA(0.0f～1.0fに正規化)
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
			srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

			device->get()->CreateShaderResourceView(resourcePtr, //ビューと関連付けるバッファ
				&srvDesc, //先ほど設定したテクスチャ設定情報
				heapHandle//ヒープのどこに割り当てるか
			);

			m_offset++;

			return true;
		}

		return false;
	}

	template<typename Value>
	inline void descriptor_heap<Value>::reset() noexcept
	{
		m_offset = 0;
	}

	template<typename Value>
	inline ID3D12DescriptorHeap*& descriptor_heap<Value>::get() noexcept
	{
		return m_descriptor_heap;
	}



}