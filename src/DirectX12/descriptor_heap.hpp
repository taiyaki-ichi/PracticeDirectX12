#pragma once
#include"device.hpp"
#include"texture_shader_resource.hpp"
#include"resource_type_tag.hpp"
#include<optional>
#include<type_traits>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

//#include<typeinfo>

namespace ichi
{
	class device;

	//ConstantBuffer用とShaderResouce用
	//sapmerはstaticにRootSignatureで設定
	class descriptor_heap
	{
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		unsigned int m_size = 0;
		unsigned int m_offset = 0;
		unsigned int m_stride = 0;
		unsigned int m_increment_size = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap();

		bool initialize(device* d, unsigned int size);

		//ビューの作製
		template<typename ResourceType>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(device* device, ID3D12Resource* resourcePtr);
		//ディスクリプタヒープをセットするようにgpu_handleを取得できるようにしておく
		//リソースタイプを持っている場合に呼ばれる関数
		template<typename T,typename = typename T::resource_type>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(device* device, T* resourcePtr);
		//持っていない場合
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(...);

		//offsetを0にする
		void reset() noexcept;

		ID3D12DescriptorHeap*& get_ptr() noexcept;

		//先頭からstride×num分進んだハンドルの位置
		//0スタート
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(unsigned int num = 0);
	};


	template<typename ResourceType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> descriptor_heap::create_view(device* device, ID3D12Resource* resourcePtr)
	{
		if (!resourcePtr)
			return std::nullopt;

		if (m_size <= m_offset)
			return std::nullopt;

		//定数バッファの場合
		if constexpr (std::is_same_v<ResourceType, constant_buffer_tag>)
		{
			auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
			heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

			//定数バッファ
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
			cbvDesc.BufferLocation = resourcePtr->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(resourcePtr->GetDesc().Width);

			//定数バッファビューの作成
			device->get()->CreateConstantBufferView(&cbvDesc, heapHandle);

			//戻り値用
			auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
			h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

			//更新
			m_offset++;

			return std::make_pair(h, heapHandle);
		}
		else if constexpr (std::is_same_v<ResourceType, shader_resource_tag>)
		{
			auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
			heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

			//テクスチャ
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = resourcePtr->GetDesc().Format;//RGBA(0.0f～1.0fに正規化)
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
			srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

			device->get()->CreateShaderResourceView(resourcePtr, //ビューと関連付けるバッファ
				&srvDesc, //先ほど設定したテクスチャ設定情報
				heapHandle//ヒープのどこに割り当てるか
			);

			//戻り値用
			auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
			h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

			//更新
			m_offset++;

			return std::make_pair(h, heapHandle);
		}
		else if constexpr (std::is_same_v<ResourceType, depth_buffer_tag>)
		{
	
			auto heapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
			heapHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(m_offset);

			//テクスチャ
			D3D12_SHADER_RESOURCE_VIEW_DESC resDesc{};
			resDesc.Format = DXGI_FORMAT_R32_FLOAT;
			resDesc.Texture2D.MipLevels = 1;
			resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

			device->get()->CreateShaderResourceView(resourcePtr, //ビューと関連付けるバッファ
				&resDesc, //先ほど設定したテクスチャ設定情報
				heapHandle//ヒープのどこに割り当てるか
			);

			//戻り値用
			auto h = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
			h.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(m_offset);

			//更新
			m_offset++;

			return std::make_pair(h, heapHandle);
		}

		std::cout << "create view is failed\n";

		return std::nullopt;
	}


	template<typename T,typename ResourceType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> descriptor_heap::create_view(device* device, T* resourcePtr)
	{
		//std::cout << typeid(ResourceType).name() << "\n";
		return create_view<ResourceType>(device, resourcePtr->get());

	}

}