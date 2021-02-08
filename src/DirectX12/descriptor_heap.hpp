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


namespace ichi
{
	class device;


	//タイプ指定用
	//初期化用の関数の定義
	//インクリメントの幅の取得用関数
	namespace descriptor_heap_type {
		struct CBV_SRV_UAV;
		struct DSV;
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

		bool initialize(device * d,unsigned int size);

		//CreateTypeにはcreate(device,handle)の静的な関数
		template<typename CreateType>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>>
			create_view(device*, ID3D12Resource*);

		//タイプを持っている場合のインターフェース関数
		//getで実際のリソースを取得できるように
		template<typename T, typename CreateType = typename T::create_view_type>
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
		//create_view_func<DescriptorHeapType,CreateType>(device,resource,cpuHandle)の作製
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
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> ichi::descriptor_heap<DescriptorHeapType>::create_view(device* device, T* resourcePtr)
	{
		return create_view<CreateType>(device, resourcePtr->get());
	}

	template<class DescriptorHeapType>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>> ichi::descriptor_heap<DescriptorHeapType>::create_view(...)
	{
		std::cout << "descriptor heap create view failed type\n";
		return std::nullopt;
	}

	//
	//ディスクリプターのタイプ用のパラメータのクラスの定義
	//


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
		//こちらもとりあえずすべてのシェーダから見えるように
		static ID3D12DescriptorHeap* initialize(device* device, int size)
		{
			ID3D12DescriptorHeap* result = nullptr;

			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
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
			device->get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}
	};



	//
	//Viewを作る関数の定義
	//

	//デフォ
	template<typename DescriptorHeapType,typename CreateType>
	inline bool create_view_func(device* device, ID3D12Resource* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
	{
		return false;
	}


	//ディスクリプタヒープのタイプがCBV_SRV_UAVの時にCBVのビューを作る
	template<>
	inline bool create_view_func<descriptor_heap_type::CBV_SRV_UAV, create_view_type::CBV>
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
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, create_view_type::SRV>
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
	inline bool  create_view_func<descriptor_heap_type::CBV_SRV_UAV, create_view_type::DSV>
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
	inline bool  create_view_func<descriptor_heap_type::DSV, create_view_type::DSV>
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


	/*

	//
	//Viewを作る時のパラメータ用のクラスの定義
	//

	//デフォ
	template<typename DescriptorHeapType>
	struct create_view_type::CBV
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle){
			return false;
		}
	};
	//ディスクリプタヒープのタイプがCBV_SRV_UAVの時
	template<>
	struct create_view_type::CBV<descriptor_heap_type::CBV_SRV_UAV>
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		{
			//定数バッファ
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
			cbvDesc.BufferLocation = resource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(resource->GetDesc().Width);

			//定数バッファビューの作成
			device->get()->CreateConstantBufferView(&cbvDesc, cpuHandle);

			return true;
		}
	};
	

	//デフォ
	template<typename DescriptorHeapType>
	struct create_view_type::SRV
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle){
			return false;
		}
	};

	template<>
	struct create_view_type::SRV<descriptor_heap_type::CBV_SRV_UAV>
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
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
	};

	//ディプス
	//デフォ
	template<typename DescriptorHeapType>
	struct create_view_type::DSV
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle){
			return false;
		}
	};
	//深度バッファをシェーダリソースとして扱うため
	//CBV_SRV_UAVのディスクリプタヒープにViewを作る用
	template<>
	struct create_view_type::DSV<descriptor_heap_type::CBV_SRV_UAV>
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
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
		}
	};

	//深度バッファを深度バッファとして使うため
	//深度バッファ用のディスクリプタヒープにViewを作る用
	template<>
	struct create_view_type::DSV<descriptor_heap_type::DSV>
	{
		static bool create_view(device* device, ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle)
		{
			//深度ビュー作成
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//フラグは特になし

			device->get()->CreateDepthStencilView(resource, //ビューと関連付けるバッファ
				&dsvDesc, //先ほど設定したテクスチャ設定情報
				cpuHandle//ヒープのどこに割り当てるか
			);

			return false;
		}
	};
	*/


	/*

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

	*/

}