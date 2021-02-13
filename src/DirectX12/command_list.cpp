#include"command_list.hpp"
#include"device.hpp"

#include<iostream>

namespace DX12
{
	command_list::~command_list()
	{
		if (m_allocator)
			m_allocator->Release();
		if (m_queue)
			m_queue->Release();
		if (m_list)
			m_list->Release();
		if (m_fence)
			m_fence->Release();
	}

	bool command_list::initialize(device* device)
	{
		//アロケータの作製
		if (FAILED(device->get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator)))) {
			std::cout << "CreateCommandAllocator is failed\n";
			return false;
		}
		
		//リスト
		if (FAILED(device->get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator, nullptr, IID_PPV_ARGS(&m_list)))) {
			std::cout << "CreateCommandList is failed\n";
			return false;
		}

		//キュー
		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
		cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//タイムアウトナシ
		cmdQueueDesc.NodeMask = 0;
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//プライオリティ特に指定なし
		cmdQueueDesc.Type = m_list->GetType();			//ここはコマンドリストと合わせる
		if (FAILED(device->get()->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_queue)))) {
			std::cout << "CreateCommandQueue is failed\n";
			return false;

		}

		//フェンス
		if (FAILED(device->get()->CreateFence(m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)))) {
			std::cout << "CreateFence is failed\n";
			return false;
		}
			
		return true;
	}

	ID3D12GraphicsCommandList* command_list::get()
	{
		return m_list;
	}

	ID3D12CommandQueue* command_list::get_queue()
	{
		return m_queue;
	}

	ID3D12CommandAllocator* command_list::get_allocator()
	{
		return m_allocator;
	}

	void command_list::execute()
	{
		m_queue->ExecuteCommandLists(1, (ID3D12CommandList**)(&m_list));
		m_queue->Signal(m_fence, ++m_fence_value);

		//コマンドキューが終了していないことの確認
		if (m_fence->GetCompletedValue() < m_fence_value)
		{
			m_fence->SetEventOnCompletion(m_fence_value, m_fence_event);
			// イベントが発火するまで待つ
			WaitForSingleObject(m_fence_event, INFINITE);
		}
	}

	void command_list::clear()
	{
		m_allocator->Reset();
		m_list->Reset(m_allocator, nullptr);
	}





}