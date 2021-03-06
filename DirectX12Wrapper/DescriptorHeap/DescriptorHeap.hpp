#pragma once
#include"DescriptorHeapTypeTag.hpp"
#include"DescriptorHeapViewHepler.hpp"
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{

	template<typename T>
	struct ViewTypeTraits;

	template<typename T>
	struct ResourcePtrTraits;

	template<typename DescriptorHeapTypeTag>
	class DescriptorHeap
	{
		ID3D12DescriptorHeap* descriptorHeap = nullptr;

		unsigned int size = 0;
		unsigned int offset = 0;
		unsigned int incrementSize = 0;

	public:
		DescriptorHeap() = default;
		~DescriptorHeap();

		DescriptorHeap(const DescriptorHeap&) = delete;
		DescriptorHeap& operator=(const DescriptorHeap&) = delete;

		DescriptorHeap(DescriptorHeap&&) noexcept;
		DescriptorHeap& operator=(DescriptorHeap&&) noexcept;

		void Initialize(Device* d, unsigned int size);

		//Viewを作り成功した場合はハンドルを返す
		//TはViewTypeTraitsとGetResourcePtrPolicyを特殊化している必要がある
		template<typename T,typename ViewType=ViewTypeTraits<T>, typename GetResourcePtrPolicy=ResourcePtrTraits<T>>
		std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>>
			PushBackView(Device* device, T* resource);

		//offsetを0にする
		void Reset() noexcept;

		ID3D12DescriptorHeap*& Get() noexcept;

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(std::size_t index = 0);
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(std::size_t index = 0);

	};


	//
	//
	//


	template<typename DescriptorHeapTypeTag>
	inline DescriptorHeap<DescriptorHeapTypeTag>::~DescriptorHeap()
	{
		if (descriptorHeap)
			descriptorHeap->Release();
	}

	template<typename DescriptorHeapTypeTag>
	inline DescriptorHeap<DescriptorHeapTypeTag>::DescriptorHeap(DescriptorHeap&& rhs) noexcept
	{
		descriptorHeap = rhs.descriptorHeap;
		size = rhs.size;
		offset = rhs.offset;
		incrementSize = rhs.incrementSize;

		rhs.descriptorHeap = nullptr;
	}

	template<typename DescriptorHeapTypeTag>
	inline DescriptorHeap<DescriptorHeapTypeTag>& DescriptorHeap<DescriptorHeapTypeTag>::operator=(DescriptorHeap&& rhs) noexcept
	{
		descriptorHeap = rhs.descriptorHeap;
		size = rhs.size;
		offset = rhs.offset;
		incrementSize = rhs.incrementSize;

		rhs.descriptorHeap = nullptr;

		return *this;
	}

	template<typename DescriptorHeapTypeTag>
	inline void DescriptorHeap<DescriptorHeapTypeTag>::Initialize(Device* d, unsigned int s)
	{
		descriptorHeap = DescriptorHeapTypeTag::Initialize(d, s);
		if (!descriptorHeap)
			throw "descriptor heap init is failed\n";

		incrementSize = DescriptorHeapTypeTag::GetIncrementSize(d);
		size = s;
	}

	template<typename DescriptorHeapTypeTag>
	template<typename T, typename ViewType,typename GetResourcePtrPolicy>
	inline std::optional<std::pair<D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE>>
		DescriptorHeap<DescriptorHeapTypeTag>::PushBackView(Device* device, T* resource)
	{
		//空いてるスペースがない場合
		if (offset >= size)
			return std::nullopt;

		//cpuハンドルの取得
		auto cpuHandle = GetCPUHandle(offset);

		//Viewを作製したいリソースのポインタを取得
		auto resourcePtr = GetResourcePtrPolicy::Get(resource);

		//viewの生成
		if (!CreateView<DescriptorHeapTypeTag, typename ViewType::Type>(device, resourcePtr, cpuHandle))
			return std::nullopt;

		//戻り値用にgpuハンドルの取得
		auto gpuHandle = GetGPUHandle(offset);

		//オフセットの更新
		offset++;

		return std::make_pair(gpuHandle, cpuHandle);
	}

	template<typename DescriptorHeapTypeTag>
	inline void DescriptorHeap<DescriptorHeapTypeTag>::Reset() noexcept
	{
		offset = 0;
	}

	template<typename DescriptorHeapTypeTag>
	inline ID3D12DescriptorHeap*& DescriptorHeap<DescriptorHeapTypeTag>::Get() noexcept
	{
		return descriptorHeap;
	}

	template<typename DescriptorHeapTypeTag>
	inline D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap<DescriptorHeapTypeTag>::GetGPUHandle(std::size_t index)
	{
		auto gpuHandle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += static_cast<UINT64>(incrementSize) * static_cast<UINT64>(index);
		return gpuHandle;
	}

	template<typename DescriptorHeapTypeTag>
	inline D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap<DescriptorHeapTypeTag>::GetCPUHandle(std::size_t index)
	{
		auto cpuHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += static_cast<SIZE_T>(incrementSize) * static_cast<SIZE_T>(index);
		return cpuHandle;
	}

}