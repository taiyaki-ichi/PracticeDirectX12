#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<memory>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace DX12
{
	class device;
	class vertex_buffer;

	//ぺらポリゴン用ルートシグネチャの作製
	std::optional<ID3D12RootSignature*> create_perapolygon_root_signature(device*);

	//ぺらポリゴン用のパイプラインステートの作製
	//引数はぺらポリゴン用のルートシグネチャ
	//２つ目はぼかし用
	std::optional<std::pair<ID3D12PipelineState*, ID3D12PipelineState*>> create_perapolygon_pipline_state(device*, ID3D12RootSignature*);

}