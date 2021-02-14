#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;
	class command_list;

	class perapolygon_renderer
	{

		ID3D12RootSignature* m_root_signature = nullptr;
		//通常のパイプライン
		ID3D12PipelineState* m_pipeline_state = nullptr;
		//ぼかし用のパイプライン
		ID3D12PipelineState* m_blur_pipeline_state = nullptr;
		//SSAO用のパイプライン
		ID3D12PipelineState* m_SSAO_pipeline_state = nullptr;

	public:
		perapolygon_renderer() = default;
		~perapolygon_renderer();

		bool initialize(device*);

		//ぺらポリゴンの描写前に呼び出す
		void preparation_for_drawing(command_list*);

		//ぺらポリゴンをぼかすための縮小されたバッファに描写する前に使用
		void preparation_for_drawing_for_blur(command_list*);
		
		//SSAOするための
		void preparation_for_drawing_for_SSAO(command_list*);

		//コピー禁止
		perapolygon_renderer(const perapolygon_renderer&) = delete;
		perapolygon_renderer& operator=(const perapolygon_renderer&) = delete;
		//ムーブ可能
		perapolygon_renderer(perapolygon_renderer&&) noexcept;
		perapolygon_renderer& operator=(perapolygon_renderer&&) noexcept;

	};


}