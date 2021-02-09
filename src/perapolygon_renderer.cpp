#include"perapolygon_renderer.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/resource_type_tag.hpp"

namespace ichi
{

	perapolygon_renderer::~perapolygon_renderer()
	{
		if (m_resource)
			m_resource->Release();
		if (m_normal_resource)
			m_normal_resource->Release();
	}

	bool perapolygon_renderer::initialize(device* device)
	{
		//doublebufferのDescの中身のぞいてうつした
		//とりあえず
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resdesc.Alignment = 65536;
		resdesc.DepthOrArraySize = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		resdesc.MipLevels = 1;
		resdesc.Height = 600;
		resdesc.Width = 800;
		resdesc.SampleDesc = { 1,0 };
		resdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		//ここで渡す値とClearの値が異なると警告出る
		//遅くなるよ、みたいな
		D3D12_CLEAR_VALUE clearValue{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.5f,0.5f,0.5f,1.f } };

		//普通の
		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,//rendertargetではない
			&clearValue,
			IID_PPV_ARGS(&m_resource)
		))) {
			std::cout << "failed perapolygon render init \n";
			return false; 
		}

		//法線用
		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,//rendertargetではない
			&clearValue,
			IID_PPV_ARGS(&m_normal_resource)
		))) {
			std::cout << "failed perapolygon render normal init \n";
			return false;
		}


		m_descriptor_heap = std::unique_ptr<descriptor_heap<descriptor_heap_type::RTV>>{
			device->create<descriptor_heap<descriptor_heap_type::RTV>>(2)
		};
		if (!m_descriptor_heap) {
			std::cout << "perapolygon init CreateDescriptorHeap is failed\n";
			return false;
		}

		m_descriptor_heap->create_view<create_view_type::RTV>(device, m_resource);
		m_descriptor_heap->create_view<create_view_type::RTV>(device, m_normal_resource);

		return true;
	}

	void perapolygon_renderer::begin_drawing(command_list* cl, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_resource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		BarrierDesc.Transition.pResource = m_normal_resource;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		//auto rtvH = m_descriptor_heap->get_cpu_handle();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvH[2] = {
			m_descriptor_heap->get_cpu_handle(0),
			m_descriptor_heap->get_cpu_handle(1)
		};
		cl->get()->OMSetRenderTargets(2, rtvH, false, &handle);
	}

	void perapolygon_renderer::end_drawing(command_list* cl)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_resource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter =  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		BarrierDesc.Transition.pResource = m_normal_resource;
		cl->get()->ResourceBarrier(1, &BarrierDesc);
	}

	void perapolygon_renderer::clear(command_list* cl)
	{
		float clearColor[] = { 0.5f,0.5f,0.5f,1.0f };
		cl->get()->ClearRenderTargetView(m_descriptor_heap->get_cpu_handle(0), clearColor, 0, nullptr);
		cl->get()->ClearRenderTargetView(m_descriptor_heap->get_cpu_handle(1), clearColor, 0, nullptr);
	}

	ID3D12Resource* perapolygon_renderer::get_resource_ptr() noexcept
	{
		return m_resource;
	}

	ID3D12Resource* perapolygon_renderer::get_normal_resource_ptr() noexcept
	{
		return m_normal_resource;
	}


}