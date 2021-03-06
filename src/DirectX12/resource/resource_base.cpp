#include"resource_base.hpp"
#include"../device.hpp"
#include"../command_list.hpp"

#include<iostream>

namespace DX12
{
	resource_base::~resource_base()
	{
		if (m_resource)
			m_resource->Release();
	}
	resource_base::resource_base(resource_base&& r) noexcept
	{
		m_resource = r.m_resource;
		m_state = r.m_state;
		r.m_resource = nullptr;
	}
	resource_base& resource_base::operator=(resource_base&& r) noexcept
	{
		m_resource = r.m_resource;
		m_state = r.m_state;
		r.m_resource = nullptr;
		return *this;
	}
	bool resource_base::initialize(device* device, const D3D12_HEAP_PROPERTIES* heapProp, D3D12_HEAP_FLAGS flag, const D3D12_RESOURCE_DESC* resoDesc, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue)
	{
		m_state = state;

		if (clearValue != nullptr)
			m_clear_value = *clearValue;


		if (SUCCEEDED(device->get()->CreateCommittedResource(
			heapProp,
			flag,
			resoDesc,
			state,
			clearValue,
			IID_PPV_ARGS(&m_resource)))) {
			return true;
		}
		else {
			std::cout << "resource initialize is failed\n";
			return false;
		}
	}

	bool resource_base::initialize(ID3D12Resource* resource)
	{
		if (resource != nullptr && is_empty()) {
			m_resource = resource;
			return true;
		}
		else
			return false;
	}

	void resource_base::barrior(command_list* cl, D3D12_RESOURCE_STATES state)
	{
		if (m_state == state)
			return;

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_resource;
		barrier.Transition.StateBefore = m_state;
		barrier.Transition.StateAfter = state;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cl->get()->ResourceBarrier(1, &barrier);
		m_state = state;
	}

	ID3D12Resource* resource_base::get() noexcept
	{
		return m_resource;
	}

	D3D12_CLEAR_VALUE* resource_base::get_clear_value()
	{
		if (m_clear_value)
			return &m_clear_value.value();
		else
			return nullptr;
	}

	bool resource_base::is_empty() const noexcept
	{
		return m_resource == nullptr;
	}
}