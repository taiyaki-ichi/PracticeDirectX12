#pragma once
#include"device.hpp"
#include"resource_type_tag.hpp"
#include<optional>
#include<type_traits>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace DX12
{
	class device;


	//タイプ指定用
	//初期化用の関数の定義
	//インクリメントの幅の取得用関数とか
	namespace descriptor_heap_type {
		struct CBV_SRV_UAV;
		struct DSV;
		struct RTV;
	};


	template<typename,typename>
	bool create_view_func(device*, ID3D12Resource*, const D3D12_CPU_DESCRIPTOR_HANDLE&);

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

		//CreateTypeにはcreate(device,handle)の静的な関数
		template<typename CreateType>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>>
			create_view(device*, ID3D12Resource*);

		//タイプを持っている場合のインターフェース関数
		//getで実際のリソースを取得できるように
		template<typename T, typename CreateType = typename T::resource_type>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(device* device, T* resourcePtr);
		//持っていない場合
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> create_view(...);

		//offsetを0にする
		void reset() noexcept {
			m_offset = 0;
		}

		//ポインタの取得
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


	template<typename DescriptorHeapType>
	template<typename CreateType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> 
		descriptor_heap<DescriptorHeapType>::create_view(device* device, ID3D12Resource* resource)
	{
		//空いてるスペースがない場合
		if (m_offset >= m_size)
			return std::nullopt;

		//cpuハンドルの取得
		auto cpuHandle = get_cpu_handle(m_offset);

		//viewの生成
		if (!create_view_func<DescriptorHeapType,CreateType>(device, resource, cpuHandle))
			return std::nullopt;

		//戻り値用にgpuハンドルの取得
		auto gpuHandle = get_gpu_handle(m_offset);

		//オフセットの更新
		m_offset++;

		return std::make_pair(gpuHandle, cpuHandle);
	}

	template<class DescriptorHeapType>
	template<typename T, typename CreateType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> DX12::descriptor_heap<DescriptorHeapType>::create_view(device* device, T* resourcePtr)
	{
		return create_view<CreateType>(device, resourcePtr->get());
	}

	template<class DescriptorHeapType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> DX12::descriptor_heap<DescriptorHeapType>::create_view(...)
	{
		std::cout << "descriptor heap create view failed type\n";
		return std::nullopt;
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




	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Viewを作る関数の定義
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//デフォ
	template<typename DescriptorHeapType,typename CreateType>
	inline bool create_view_func(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		return false;
	}


	//ディスクリプタヒープのタイプがCBV_SRV_UAVの時にCBVのビューを作る
	template<>
	inline bool create_view_func<descriptor_heap_type::CBV_SRV_UAV, resource_type::CBV>
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
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, resource_type::SRV>
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
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, resource_type::DSV>
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
	inline bool  create_view_func<descriptor_heap_type::DSV, resource_type::DSV>
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
	inline bool create_view_func<descriptor_heap_type::RTV, resource_type::RTV>
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
	inline bool create_view_func<descriptor_heap_type::RTV, resource_type::R32>
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
	inline bool create_view_func<descriptor_heap_type::CBV_SRV_UAV, resource_type::R32>
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

}