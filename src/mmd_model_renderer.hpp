#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;
	class command_list;

	class mmd_model_renderer
	{
		//通常のパイプラインステート
		ID3D12PipelineState* m_pipeline_state = nullptr;
		//シャドウ用のパイプラインステート
		ID3D12PipelineState* m_light_depth_pipeline_state = nullptr;
		//ルートシグネチャ
		ID3D12RootSignature* m_root_signature = nullptr;

	public:
		mmd_model_renderer() = default;
		~mmd_model_renderer();

		bool initialize(device*);

		//mmd_modelを描写する前に呼び出す
		void preparation_for_drawing(command_list*);
		
		//ライト深度を描写する前に呼び出す
		void preparation_for_drawing_light_depth(command_list*);


		//コピー禁止
		mmd_model_renderer(const mmd_model_renderer&) = delete;
		mmd_model_renderer& operator=(const mmd_model_renderer&) = delete;
		//ムーブ可能
		mmd_model_renderer(mmd_model_renderer&&) noexcept;
		mmd_model_renderer& operator=(mmd_model_renderer&&) noexcept;
	};

}