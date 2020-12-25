#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace ichi
{
	class device;

	class pipeline_state
	{
		ID3D12PipelineState* m_pipeline_state = nullptr;
		ID3D12RootSignature* m_root_signature = nullptr;

	public:
		pipeline_state() = default;
		~pipeline_state();
	
		bool initialize(device*, ID3DBlob* vertexShader, ID3DBlob* pixelShader);

		ID3D12PipelineState* get() const noexcept;
		ID3D12RootSignature* get_root_signature() const noexcept;

	};
}