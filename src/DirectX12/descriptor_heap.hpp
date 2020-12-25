#pragma once
#include"device.hpp"
#include"texture_shader_resource.hpp"
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

	//ConstantBuffer用とShaderResouce用
	//sapmerはstaticにRootSignatureで設定
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
		//deviceを引数に取るのいやだな
		template<typename Value>
		bool create_view(device* device, Value* resourcePtr);

		//offsetを0にする
		void reset() noexcept;

		ID3D12DescriptorHeap*& get() noexcept;
	};



	template<typename Value>
	inline bool descriptor_heap::create_view(device* device, Value* resourcePtr)
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

			device->get()->CreateShaderResourceView(resourcePtr->get(), //ビューと関連付けるバッファ
				&srvDesc, //先ほど設定したテクスチャ設定情報
				heapHandle//ヒープのどこに割り当てるか
			);

			m_offset++;

			return true;
		}

		return false;
	}


}