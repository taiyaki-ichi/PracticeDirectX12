#pragma once
#include"root_signature/root_signature.hpp"
#include"format.hpp"
#include"shader.hpp"
#include"device.hpp"
#include"primitive_topology.hpp"
#include"resource/vertex_buffer_resource.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<vector>
#include<array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	struct ShaderDesc {
		shader* vertexShader = nullptr;
		shader* pixcelShader = nullptr;
		shader* geometryShader = nullptr;
		shader* hullShader = nullptr;
		shader* domainShader = nullptr;
	};


	template<typename VertexFormatTuple,typename RenderTargetFormatTuple>
	class graphics_pipeline_state
	{
		release_unique_ptr<ID3D12PipelineState> pipeline_state_ptr{};

	public:
		graphics_pipeline_state() = default;
		~graphics_pipeline_state() = default;

		graphics_pipeline_state(graphics_pipeline_state&&) = default;
		graphics_pipeline_state& operator=(graphics_pipeline_state&&) = default;

		void initialize(device&, root_signature&, ShaderDesc,
			std::array<const char*,VertexFormatTuple::get_formats_num()> vertexName,
			bool depthEnable, bool alphaBlend, primitive_topology primitiveTopology
		);

		ID3D12PipelineState* get() const noexcept;

	};


	class compute_pipeline_state
	{
		release_unique_ptr<ID3D12PipelineState> pipeline_state_ptr{};

	public:
		compute_pipeline_state() = default;
		~compute_pipeline_state() = default;

		compute_pipeline_state(compute_pipeline_state&&) = default;
		compute_pipeline_state& operator=(compute_pipeline_state&&) = default;

		//コンピュートシェーダ用のパイプライン生成
		void initialize(device&, root_signature&, shader& computeShader);

		ID3D12PipelineState* Get() const noexcept;
	};


	//
	//ヘルパ
	//

	template<std::size_t I, typename VertexFormatTuple>
	inline constexpr void get_input_element_desc_impl(std::array<const char*, VertexFormatTuple::get_formats_num()>& vertexName,
		std::array<D3D12_INPUT_ELEMENT_DESC, VertexFormatTuple::get_formats_num()>& result)
	{
		result[I].SemanticName = vertexName[I];
		result[I].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		result[I].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		result[I].Format = VertexFormatTuple::template get_dxgi_format<I>();

		if constexpr (I + 1 < VertexFormatTuple::get_formats_num())
			get_input_element_desc_impl<I + 1, VertexFormatTuple>(vertexName, result);
	}

	template<typename VertexFormatTuple>
	inline constexpr std::array<D3D12_INPUT_ELEMENT_DESC, VertexFormatTuple::get_formats_num()> get_input_element_desc(std::array<const char*, VertexFormatTuple::get_formats_num()>& vertexName)
	{
		std::array<D3D12_INPUT_ELEMENT_DESC, VertexFormatTuple::get_formats_num()> result{};
		get_input_element_desc_impl<0, VertexFormatTuple>(vertexName, result);
		return result;
	}

	template<std::size_t I, typename RenderTargetFormatTuple>
	inline constexpr void get_render_target_format(DXGI_FORMAT(&result)[8])
	{
		result[I] = RenderTargetFormatTuple::template get_dxgi_format<I>();

		if constexpr (I + 1 < RenderTargetFormatTuple::get_formats_num())
			get_render_target_format<I + 1, RenderTargetFormatTuple>(result);
	}


	//
	//
	//

	template<typename VertexFormatTuple, typename RenderTargetFormatTuple>
	inline void graphics_pipeline_state<VertexFormatTuple, RenderTargetFormatTuple>::initialize(device& device, root_signature& rootSignature, ShaderDesc shaderDesc, 
		std::array<const char*, VertexFormatTuple::get_formats_num()> vertexName, bool depthEnable, bool alphaBlend, primitive_topology primitiveTopology)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		if (shaderDesc.vertexShader) {
			graphicsPipelineDesc.VS.pShaderBytecode = shaderDesc.vertexShader->get()->GetBufferPointer();
			graphicsPipelineDesc.VS.BytecodeLength = shaderDesc.vertexShader->get()->GetBufferSize();
		}
		if (shaderDesc.pixcelShader) {
			graphicsPipelineDesc.PS.pShaderBytecode = shaderDesc.pixcelShader->get()->GetBufferPointer();
			graphicsPipelineDesc.PS.BytecodeLength = shaderDesc.pixcelShader->get()->GetBufferSize();
		}
		if (shaderDesc.geometryShader) {
			graphicsPipelineDesc.GS.pShaderBytecode = shaderDesc.geometryShader->get()->GetBufferPointer();
			graphicsPipelineDesc.GS.BytecodeLength = shaderDesc.geometryShader->get()->GetBufferSize();
		}
		if (shaderDesc.hullShader) {
			graphicsPipelineDesc.HS.pShaderBytecode = shaderDesc.hullShader->get()->GetBufferPointer();
			graphicsPipelineDesc.HS.BytecodeLength = shaderDesc.hullShader->get()->GetBufferSize();
		}
		if (shaderDesc.domainShader) {
			graphicsPipelineDesc.DS.pShaderBytecode = shaderDesc.domainShader->get()->GetBufferPointer();
			graphicsPipelineDesc.DS.BytecodeLength = shaderDesc.domainShader->get()->GetBufferSize();
		}

		auto inputElementDescs = get_input_element_desc<VertexFormatTuple>(vertexName);

		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputElementDescs.data();
		graphicsPipelineDesc.InputLayout.NumElements = inputElementDescs.size();
		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//カット値はナシ
		graphicsPipelineDesc.PrimitiveTopologyType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(primitiveTopology);


		//ラスタライザの設定
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		//カリングはしない。つまり面の向きが裏でも描写する
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.ForcedSampleCount = 0;
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		graphicsPipelineDesc.RasterizerState = rasterizerDesc;


		//ブレンドステート
		D3D12_RENDER_TARGET_BLEND_DESC renderTagetBlendDesc{};
		if (alphaBlend)
		{
			renderTagetBlendDesc.BlendEnable = TRUE;
			renderTagetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
			renderTagetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			renderTagetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;;
			renderTagetBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
			renderTagetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			renderTagetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			renderTagetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			renderTagetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
			renderTagetBlendDesc.LogicOpEnable = FALSE;
		}
		else {
			renderTagetBlendDesc.BlendEnable = FALSE;//とりあえず
			renderTagetBlendDesc.LogicOpEnable = FALSE;
			renderTagetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}

		D3D12_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = TRUE;
		blendDesc.IndependentBlendEnable = FALSE;
		//renderTagetBlendDesc.BlendEnableがFalseなので1つ目のみ設定で十分
		blendDesc.RenderTarget[0] = renderTagetBlendDesc;

		graphicsPipelineDesc.BlendState = blendDesc;

		graphicsPipelineDesc.NumRenderTargets = RenderTargetFormatTuple::get_formats_num();
		if constexpr (RenderTargetFormatTuple::get_formats_num() > 0)
			get_render_target_format<0, RenderTargetFormatTuple>(graphicsPipelineDesc.RTVFormats);

		//デプスステンシル
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = depthEnable;
		if (depthEnable) {
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			graphicsPipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		}
		depthStencilDesc.StencilEnable = FALSE;

		graphicsPipelineDesc.DepthStencilState = depthStencilDesc;

		//サンプルディスク
		graphicsPipelineDesc.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
		graphicsPipelineDesc.SampleDesc.Quality = 0;//クオリティは最低

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//これないとまずい

		graphicsPipelineDesc.pRootSignature = rootSignature.Get();

		{
			ID3D12PipelineState* tmp = nullptr;
			if (FAILED(device.get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&tmp))))
				THROW_EXCEPTION("");
			pipeline_state_ptr.reset(tmp);
		}
	}

	template<typename VertexLayout, typename ResnderTargetFormats>
	inline ID3D12PipelineState* graphics_pipeline_state<VertexLayout, ResnderTargetFormats>::get() const noexcept
	{
		return pipeline_state_ptr.get();
	}

	inline void compute_pipeline_state::initialize(device& device, root_signature& rootSignature, shader& computeShader)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
		computePipelineDesc.CS.pShaderBytecode = computeShader.get()->GetBufferPointer();
		computePipelineDesc.CS.BytecodeLength = computeShader.get()->GetBufferSize();
		computePipelineDesc.pRootSignature = rootSignature.Get();

		{
			ID3D12PipelineState* tmp = nullptr;
			if (FAILED(device.get()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&tmp))))
				THROW_EXCEPTION("");
			pipeline_state_ptr.reset(tmp);
		}
	}

	inline ID3D12PipelineState* compute_pipeline_state::Get() const noexcept
	{
		return pipeline_state_ptr.get();
	}
}