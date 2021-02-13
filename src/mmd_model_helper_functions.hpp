#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace DX12
{
	class device;

	//mmd用のルートシグネチャの作製
	std::optional<ID3D12RootSignature*> create_mmd_rootsignature(device*);

	//mmd用のパイプラインステートの作製
	//前者は普通の、後者はシャドウ用の
	std::optional<std::pair<ID3D12PipelineState*, ID3D12PipelineState*>> create_mmd_pipline_state(device*, ID3D12RootSignature*);

}