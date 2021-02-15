#include"depth_stencil_buffer.hpp"

namespace DX12
{
	bool depth_stencil_buffer::initialize(device* device, unsigned int width, unsigned int height)
	{
		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元のテクスチャデータとして
		depthResDesc.Width = width;
		depthResDesc.Height = height;
		depthResDesc.DepthOrArraySize = 1;//テクスチャ配列でもないし3Dテクスチャでもない
		depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;//深度値書き込み用フォーマット
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
		depthClearValue.DepthStencil.Depth = 1.f;//深さ１(最大値)でクリア
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit深度値としてクリア
		depthClearValue.Color[0] = 1.f;
		depthClearValue.Color[1] = 1.f;
		depthClearValue.Color[2] = 1.f;
		depthClearValue.Color[3] = 1.f;

		return resource_base::initialize(device,
			&depthHeapProp, 
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
			&depthClearValue
		);

	}

}