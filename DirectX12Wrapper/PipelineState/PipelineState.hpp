#pragma once
#include"../RootSignature/RootSignature.hpp"
#include"VertexLayout.hpp"
#include"../Format.hpp"
#include"../Shader.hpp"
#include"../Device.hpp"
#include"../PrimitiveTopology.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<vector>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	struct ShaderDesc {
		Shader* vertexShader = nullptr;
		Shader* pixcelShader = nullptr;
		Shader* geometryShader = nullptr;
		Shader* hullShader = nullptr;
		Shader* domainShader = nullptr;
	};
	
	class PipelineState
	{
		ID3D12PipelineState* pipelineState = nullptr;
		//参照
		ID3D12RootSignature* rootSignature = nullptr;

	public:
		PipelineState() = default;
		~PipelineState();

		PipelineState(const  PipelineState&) = delete;
		PipelineState& operator=(const PipelineState&) = delete;

		PipelineState(PipelineState&&) noexcept;
		PipelineState& operator=(PipelineState&&) noexcept;

		void Initialize(Device*, RootSignature*, ShaderDesc,
			const std::vector<std::pair<std::string,FFormat>>& vertexLayouts, const std::vector<FFormat>& renderTargetFormats,
			bool depthEnable, bool alphaBlend, PrimitiveTopology primitiveTopology
		);

		//コンピュートシェーダ用のパイプライン生成
		void Initialize(Device*, RootSignature*, Shader* computeShader);

		ID3D12PipelineState* Get() const noexcept;
		ID3D12RootSignature* GetRootSignature() const noexcept;

	};


	//
	//
	//

	PipelineState::~PipelineState() {
		if (pipelineState)
			pipelineState->Release();
	}

	inline PipelineState::PipelineState(PipelineState&& rhs) noexcept {
		pipelineState = rhs.pipelineState;
		rhs.pipelineState = nullptr;
	}
	inline PipelineState& PipelineState::operator=(PipelineState&& rhs) noexcept {
		pipelineState = rhs.pipelineState;
		rhs.pipelineState = nullptr;
		return *this;
	}
	
	inline void PipelineState::Initialize(Device* device, RootSignature* rootSignature, ShaderDesc shaderDesc, const std::vector<std::pair<std::string, FFormat>>& vertexLayouts, 
		const std::vector<FFormat>& renderTargetFormats, bool depthEnable, bool alphaBlend, PrimitiveTopology primitiveTopology)
	{
		this->rootSignature = rootSignature->Get();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};


		if (!shaderDesc.vertexShader || !shaderDesc.pixcelShader)
			throw "";
		graphicsPipelineDesc.VS.pShaderBytecode = shaderDesc.vertexShader->Get()->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = shaderDesc.vertexShader->Get()->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = shaderDesc.pixcelShader->Get()->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = shaderDesc.pixcelShader->Get()->GetBufferSize();
		if (shaderDesc.geometryShader) {
			graphicsPipelineDesc.GS.pShaderBytecode = shaderDesc.geometryShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.GS.BytecodeLength = shaderDesc.geometryShader->Get()->GetBufferSize();
		}
		if (shaderDesc.hullShader) {
			graphicsPipelineDesc.HS.pShaderBytecode = shaderDesc.hullShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.HS.BytecodeLength = shaderDesc.hullShader->Get()->GetBufferSize();
		}
		if (shaderDesc.domainShader) {
			graphicsPipelineDesc.DS.pShaderBytecode = shaderDesc.domainShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.DS.BytecodeLength = shaderDesc.domainShader->Get()->GetBufferSize();
		}

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs(vertexLayouts.size(), D3D12_INPUT_ELEMENT_DESC{});
		std::uint32_t alignedByteOffset = 0;
		for (std::size_t i = 0; i < vertexLayouts.size(); i++)
		{
			inputElementDescs[i].SemanticName = vertexLayouts[i].first.data();
			inputElementDescs[i].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			inputElementDescs[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			inputElementDescs[i].Format = vertexLayouts[i].second.value;
		}

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
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;//シャドウマップするときは変更したい
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


		graphicsPipelineDesc.NumRenderTargets = renderTargetFormats.size();
		for (std::size_t i = 0; i < renderTargetFormats.size(); i++)
			graphicsPipelineDesc.RTVFormats[i] = renderTargetFormats[i].value;


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

		graphicsPipelineDesc.pRootSignature = rootSignature->Get();

		if (FAILED(device->Get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&pipelineState))))
			throw "";

	}
	

	inline void PipelineState::Initialize(Device* device, RootSignature* rootSignature, Shader* computeShader)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
		computePipelineDesc.CS.pShaderBytecode = computeShader->Get()->GetBufferPointer();
		computePipelineDesc.CS.BytecodeLength = computeShader->Get()->GetBufferSize();
		computePipelineDesc.pRootSignature = rootSignature->Get();

		if (FAILED(device->Get()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&pipelineState))))
			throw "";
	}

	inline ID3D12PipelineState* PipelineState::Get() const noexcept
	{
		return pipelineState;
	}

	inline ID3D12RootSignature* PipelineState::GetRootSignature() const noexcept
	{
		return rootSignature;
	}
}