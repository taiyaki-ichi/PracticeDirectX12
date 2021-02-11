#include"resource.hpp"
#include"device.hpp"
#include"command_list.hpp"

namespace ichi
{

	resource::~resource()
	{
		if (m_resource)
			m_resource->Release();
	}

	bool resource::initialize(device* device, const D3D12_HEAP_PROPERTIES* heapProp, D3D12_HEAP_FLAGS flag,
		const D3D12_RESOURCE_DESC* resoDesc, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue)
	{
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

	void resource::barrior(command_list* cl, D3D12_RESOURCE_STATES state)
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

	ID3D12Resource* resource::get() noexcept
	{
		return m_resource;
	}

	bool resource::is_empty() const noexcept
	{
		return m_resource == nullptr;
	}
}