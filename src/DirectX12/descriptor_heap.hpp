#pragma once
#include"device.hpp"
#include"descriptor_heap_type.hpp"
#include"descriptor_heap_hepler_func.hpp"
#include<optional>
#include<type_traits>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace DX12
{
	class device;


	//タイプはテンプレートで指定
	template<class DescriptorHeapType>
	class descriptor_heap
	{
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		unsigned int m_size = 0;
		unsigned int m_offset = 0;
		unsigned int m_increment_size = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap() {
			if (m_descriptor_heap)
				m_descriptor_heap->Release();
		}

		//コピー禁止
		descriptor_heap<DescriptorHeapType>(const descriptor_heap<DescriptorHeapType>&) = delete;
		descriptor_heap<DescriptorHeapType>& operator=(const descriptor_heap<DescriptorHeapType>&) = delete;
		//ムーブ可能
		descriptor_heap<DescriptorHeapType>(descriptor_heap<DescriptorHeapType>&&) noexcept;
		descriptor_heap<DescriptorHeapType>& operator=(descriptor_heap<DescriptorHeapType>&&) noexcept;


		bool initialize(device * d,unsigned int size);


		//Viewの生成
		template<typename T,typename ViewType=typename DefaultViewTypeTraits<T>::view_type,typename GetResourcePtr=GetResourcePtrPolicy<T>>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>>
			create_view(device* device, T* resource);


		//offsetを0にする
		void reset() noexcept {
			m_offset = 0;
		}


		//ディスクリプタヒープのポインタの取得
		ID3D12DescriptorHeap*& get() noexcept {
			return m_descriptor_heap;
		}


		//先頭からstride×num分進んだハンドルの位置
		//0スタート
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(unsigned int num = 0) {
			auto gpuHandle = m_descriptor_heap->GetGPUDescriptorHandleForHeapStart();
			gpuHandle.ptr += static_cast<UINT64>(m_increment_size) * static_cast<UINT64>(num);
			return gpuHandle;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_handle(unsigned int num = 0) {
			auto cpuHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
			cpuHandle.ptr += static_cast<size_t>(m_increment_size) * static_cast<size_t>(num);
			return cpuHandle;
		}
	};


	//
	//クラステンプレートの実装
	//

	//初期化
	template<typename DescriptorHeapType>
	inline bool descriptor_heap<DescriptorHeapType>::initialize(device* d, unsigned int size)
	{
		m_descriptor_heap = DescriptorHeapType::initialize(d, size);
		if (!m_descriptor_heap) {
			std::cout << "descriptor heap init is failed\n";
			return false;
		}
		m_increment_size = DescriptorHeapType::get_increment_size(d);
		m_size = size;

		return true;
	}

	//Viewの生成
	template<class DescriptorHeapType>
	template<typename T, typename ViewType,typename GetResourcePtr>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> DX12::descriptor_heap<DescriptorHeapType>::create_view(device* device, T* resource)
	{
		//空いてるスペースがない場合
		if (m_offset >= m_size)
			return std::nullopt;

		//cpuハンドルの取得
		auto cpuHandle = get_cpu_handle(m_offset);

		//Viewを作製したいリソースのポインタを取得
		auto resourcePtr = GetResourcePtr::get_resource_ptr(resource);

		//viewの生成
		if (!create_view_func<DescriptorHeapType, ViewType>(device, resourcePtr, cpuHandle))
			return std::nullopt;

		//戻り値用にgpuハンドルの取得
		auto gpuHandle = get_gpu_handle(m_offset);

		//オフセットの更新
		m_offset++;

		return std::make_pair(gpuHandle, cpuHandle);
	}


	//
	//ムーブ
	//
	template<class DescriptorHeapType>
	inline DX12::descriptor_heap<DescriptorHeapType>::descriptor_heap(descriptor_heap<DescriptorHeapType>&& r) noexcept
	{
		m_descriptor_heap = r.m_descriptor_heap;;
		m_size = r.m_size;
		m_offset = r.m_offset;
		m_increment_size = r.m_increment_size;
		r.m_descriptor_heap = nullptr;
	}

	template<class DescriptorHeapType>
	inline descriptor_heap<DescriptorHeapType>& descriptor_heap<DescriptorHeapType>::operator=(descriptor_heap<DescriptorHeapType>&& r) noexcept
	{
		m_descriptor_heap = r.m_descriptor_heap;;
		m_size = r.m_size;
		m_offset = r.m_offset;
		m_increment_size = r.m_increment_size;
		r.m_descriptor_heap = nullptr;
		return *this;
	}




	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//ディスクリプターのタイプ用のパラメータのクラスの定義
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//定数バッファとシェーダ理ソールと他１用
	struct descriptor_heap_type::CBV_SRV_UAV
	{
		//とりあえずすべてのシェーダから見えるようにしておく
		//後でカスタムできるようになるかも
		static ID3D12DescriptorHeap* initialize(device* device,int size)
		{
			ID3D12DescriptorHeap* result = nullptr;

			D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
			descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			descHeapDesc.NodeMask = 0;
			descHeapDesc.NumDescriptors = size;
			descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	
			if (FAILED(device->get()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&result))))
				return nullptr;
			else
				return result;
		}

		static unsigned int get_increment_size(device* device)
		{
			return device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	};


	//ディプス用
	struct descriptor_heap_type::DSV
	{
		//こちらもとりあえずすべてのシェーダから見えるようにしたらエラーはいた
		static ID3D12DescriptorHeap* initialize(device* device, int size)
		{
			ID3D12DescriptorHeap* result = nullptr;

			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
			dsvHeapDesc.NodeMask = 0;
			dsvHeapDesc.NumDescriptors = size;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			if (FAILED(device->get()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&result))))
				return nullptr;
			else
				return result;
		}

		static unsigned int get_increment_size(device* device)
		{
			return device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}
	};


	//レンダーターゲット用
	struct descriptor_heap_type::RTV
	{
		static ID3D12DescriptorHeap* initialize(device* device, int size)
		{
			ID3D12DescriptorHeap* result = nullptr;

			D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビューなので当然RTV
			heapDesc.NodeMask = 0;
			heapDesc.NumDescriptors = size;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//特に指定なし
			if (FAILED(device->get()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&result))))
				return nullptr;
			else
				return result;
		}

		static unsigned int get_increment_size(device* device)
		{
			return device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

	};



}