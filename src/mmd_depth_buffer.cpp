#include"mmd_depth_buffer.hpp"
#include"DirectX12/device.hpp"

namespace ichi
{
	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>>
		create_mmd_depth_buffers(device* device, unsigned int windowWidth, unsigned int windowHeight, unsigned int shadowDifinition)
	{

		ID3D12Resource* depthBuffer = nullptr;

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
			IID_PPV_ARGS(&depthBuffer)
		))) {
			std::cout << "mmd depth_buffer initialize is failed\n";
			return std::nullopt;
		}


		ID3D12Resource* lightDepthBuffer = nullptr;

		depthResDesc.Width = shadowDifinition;
		depthResDesc.Height = shadowDifinition;
		depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;

		if (FAILED(device->get()->CreateCommittedResource(
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
			&depthClearValue,
			IID_PPV_ARGS(&lightDepthBuffer)
		))) {
			std::cout << "mmd light depth initialize is failed\n";
			if (depthBuffer)
				depthBuffer->Release();
			return std::nullopt;
		}


		return std::make_pair(depthBuffer, lightDepthBuffer);
	}


}