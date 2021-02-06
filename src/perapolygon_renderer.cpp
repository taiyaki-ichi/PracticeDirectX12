#include"perapolygon_renderer.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/depth_buffer.hpp"


namespace ichi
{

	perapolygon_renderer::~perapolygon_renderer()
	{
		if (m_descriptor_heap)
			m_descriptor_heap->Release();
		if (m_resource)
			m_resource->Release();
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

		//たぶんできてる
		//ここで渡す値とClearの値が異なると警告出る
		//遅くなるよ、みたいな
		D3D12_CLEAR_VALUE clearValue{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 1.f,1.f,1.f,1.f } };

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


		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビューなので当然RTV
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = 1;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//特に指定なし
		if (FAILED(device->get()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptor_heap)))) {
			std::cout << "perapolygon init CreateDescriptorHeap is failed\n";
			return false;
		}

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		device->get()->CreateRenderTargetView(m_resource, &rtvDesc, m_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

		return true;
	}

	void perapolygon_renderer::begin_drawing(command_list* cl, depth_buffer* db)
	{

		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = m_resource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		cl->get()->ResourceBarrier(1, &BarrierDesc);

		auto descPtr = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		auto bdDescPtr = db->get_cpu_descriptor_handle();
		cl->get()->OMSetRenderTargets(1, &descPtr,false, &bdDescPtr);

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
	}

	void perapolygon_renderer::clear(command_list* cl)
	{
		float clearColor[] = { 1.f,1.f,1.f,1.0f };
		cl->get()->ClearRenderTargetView(m_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
	}

	ID3D12Resource* perapolygon_renderer::get() noexcept
	{
		return m_resource;
	}



}