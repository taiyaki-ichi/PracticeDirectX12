#pragma once
#include"../Device.hpp"
#include"../CommandList.hpp"
#include"ResourceState.hpp"
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{

	//全てのリソースの元
	class ResourceBase
	{
		ID3D12Resource* resource = nullptr;

		ResourceState state{};

		std::optional<D3D12_CLEAR_VALUE> clearValue{};

	public:
		ResourceBase() = default;
		virtual ~ResourceBase();

		ResourceBase(const ResourceBase&) = delete;
		ResourceBase& operator=(const ResourceBase&) = delete;

		ResourceBase(ResourceBase&&) noexcept;
		ResourceBase& operator=(ResourceBase&&) noexcept;

		void Initialize(Device* device, const D3D12_HEAP_PROPERTIES* heapProp, D3D12_HEAP_FLAGS flag,
			const D3D12_RESOURCE_DESC* resoDesc, ResourceState s, const D3D12_CLEAR_VALUE* cv);

		//主にバックバッファを生成する際に使用
		//なくしてしまいたい
		void Initialize(ID3D12Resource* r);

		void Barrior(CommandList*, ResourceState);

		ID3D12Resource* Get() const noexcept;

		D3D12_CLEAR_VALUE* GetClearValue();
	};


	//
	//
	//


	inline ResourceBase::~ResourceBase()
	{
		if (resource)
			resource->Release();
	}

	inline ResourceBase::ResourceBase(ResourceBase&& rhs) noexcept
	{
		resource = rhs.resource;
		state = rhs.state;
		clearValue = std::move(rhs.clearValue);

		rhs.resource = nullptr;
	}

	inline ResourceBase& ResourceBase::operator=(ResourceBase&& rhs) noexcept
	{
		resource = rhs.resource;
		state = rhs.state;
		clearValue = std::move(rhs.clearValue);

		rhs.resource = nullptr;

		return *this;
	}

	inline void ResourceBase::Initialize(Device* device, const D3D12_HEAP_PROPERTIES* heapProp, D3D12_HEAP_FLAGS flag,
		const D3D12_RESOURCE_DESC* resoDesc, ResourceState s, const D3D12_CLEAR_VALUE* cv)
	{
		state = s;

		if (cv != nullptr)
			clearValue = *cv;

		if (FAILED(device->Get()->CreateCommittedResource(
			heapProp,
			flag,
			resoDesc,
			static_cast<D3D12_RESOURCE_STATES>(s),
			cv,
			IID_PPV_ARGS(&resource))))
		{
			throw  "resource initialize is failed\n";
		}
	}

	inline void ResourceBase::Initialize(ID3D12Resource* r)
	{
		if (resource == nullptr && r != nullptr)
			resource = r;
		else
			throw "";
	}

	inline void ResourceBase::Barrior(CommandList* cl, ResourceState rs)
	{
		if (state == rs)
			return;


		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(state);
		barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(rs);
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cl->Get()->ResourceBarrier(1, &barrier);

		state = rs;
	}

	inline ID3D12Resource* ResourceBase::Get() const noexcept
	{
		return resource;
	}

	inline D3D12_CLEAR_VALUE* ResourceBase::GetClearValue()
	{
		if (clearValue)
			return &clearValue.value();
		else
			return nullptr;
	}

}