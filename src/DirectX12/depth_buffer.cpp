#include"depth_buffer.hpp"
#include"device.hpp"
#include"command_list.hpp"

namespace ichi
{
	depth_buffer::~depth_buffer()
	{
		if (m_resource)
			m_resource->Release();
		if (m_descriptor_heap)
			m_descriptor_heap->Release();
	}
	bool depth_buffer::initialize(device* device,unsigned int windowWidth,unsigned int windowHeight)
	{
		//深度バッファの仕様
		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元のテクスチャデータとして
		depthResDesc.Width = windowWidth;//幅と高さはレンダーターゲットと同じ
		depthResDesc.Height = windowHeight;//上に同じ
		depthResDesc.DepthOrArraySize = 1;//テクスチャ配列でもないし3Dテクスチャでもない
		depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;//深度値書き込み用フォーマット
		depthResDesc.SampleDesc.Count = 1;//サンプルは1ピクセル当たり1つ
		depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//このバッファは深度ステンシルとして使用します
		depthResDesc.MipLevels = 1;
		depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthResDesc.Alignment = 0;

		//デプス用ヒーププロパティ
		D3D12_HEAP_PROPERTIES depthHeapProp{};
		depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULTだから後はUNKNOWNでよし
		depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//このクリアバリューが重要な意味を持つ
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.0f;//深さ１(最大値)でクリア
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit深度値としてクリア

		if (FAILED(device->get()->CreateCommittedResource(
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
			&depthClearValue,
			IID_PPV_ARGS(&m_resource)
		))) {
			std::cout << "depth_buffer initialize is failed\n";
			return false;
		}

		//深度のためのデスクリプタヒープ作成
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};//深度に使うよという事がわかればいい
		dsvHeapDesc.NumDescriptors = 1;//深度ビュー1つのみ
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//デプスステンシルビューとして使う
		if (FAILED(device->get()->CreateDescriptorHeap(
			&dsvHeapDesc, IID_PPV_ARGS(&m_descriptor_heap)))) {
			std::cout << "depth_buffer descriptor is failed\n";
			return false;
		}
		
		//深度ビュー作成
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//フラグは特になし

		device->get()->CreateDepthStencilView(
			m_resource, &dsvDesc, m_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

		return true;
	}

	void depth_buffer::clear(command_list* cl)
	{
		cl->get()->ClearDepthStencilView(
			m_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
			D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE depth_buffer::get_cpu_descriptor_handle()
	{
		return m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
	}
	ID3D12Resource* depth_buffer::get()
	{
		return m_resource;
	}
}