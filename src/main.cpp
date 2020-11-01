#include"window.hpp"
#include<iostream>
#include"DirectX12.hpp"

int main()
{
	auto hwnd = graphics::create_window(L"aaaa", 400, 300);

	using namespace graphics;

	init_debug();
	auto factory = create_dxgi_factory();
	auto adapter = get_adapter(factory);
	auto device = create_device(adapter);
	auto allocator = create_command_allocator(device);
	auto list = create_command_list(device, allocator);
	auto queue = create_command_queue(device, list);
	auto swapChain = create_swap_chain(factory, hwnd, queue);
	auto descriptorHeap = create_descriptor_heap(device);
	auto buffers = create_buffers(device, swapChain, descriptorHeap);
	auto [fence, fenceVal] = create_fence(device);

	while (graphics::process_window_message())
	{
		auto bbIdx = swapChain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = buffers[bbIdx].Get();
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		list->ResourceBarrier(1, &BarrierDesc);

		//レンダーターゲットを指定
		auto rtvH = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		list->OMSetRenderTargets(1, &rtvH, false, nullptr);

		//画面クリア
		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//黄色
		list->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		list->ResourceBarrier(1, &BarrierDesc);

		//命令のクローズ
		list->Close();


		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { list.Get() };
		queue->ExecuteCommandLists(1, cmdlists);
		////待ち
		queue->Signal(fence.Get(), ++fenceVal);

		if (fence->GetCompletedValue() != fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		allocator->Reset();//キューをクリア
		list->Reset(allocator.Get(), nullptr);//再びコマンドリストをためる準備


		//フリップ
		swapChain->Present(1, 0);
	}

	UnregisterClass(L"aaaa", GetModuleHandle(nullptr));

	return 0;
}