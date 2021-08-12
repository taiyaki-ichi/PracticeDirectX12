#pragma once
#include"../Device.hpp"
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

		void Initialize(ID3D12Resource* r);

		ID3D12Resource* Get() const noexcept;

		D3D12_CLEAR_VALUE* GetClearValue();

		const ResourceState GetState() const noexcept;
		void SetState(ResourceState) noexcept;
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
			throw  "";
		}
	}

	inline void ResourceBase::Initialize(ID3D12Resource* r)
	{
		if (resource == nullptr && r != nullptr)
			resource = r;
		else
			throw "";
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

	inline const ResourceState ResourceBase::GetState() const noexcept
	{
		return state;
	}

	inline void ResourceBase::SetState(ResourceState s) noexcept
	{
		state = s;
	}

}