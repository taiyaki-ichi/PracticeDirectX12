#pragma once
#include"../RootSignature/RootSignature.hpp"
#include"VertexLayout.hpp"
#include"../Format.hpp"
#include"../Shader.hpp"
#include"../Device.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<vector>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	
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

		void Initialize(Device*, RootSignature*, Shader* vertexShader, Shader* pixcelShader,
			const std::vector<VertexLayout>&, const std::vector<Format>&, bool depthEnable,
			Shader* geometrtShader = nullptr,
			Shader* hullShader = nullptr, Shader* domainShader = nullptr);

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

	inline void PipelineState::Initialize(Device* device, RootSignature* rootSignature, 
		 Shader* vertexShader, Shader* pixcelShader, const std::vector<VertexLayout>& vertexLayouts, 
		const std::vector<Format>& renderTargetFormats,bool depthEnable, Shader* geometrtShader,
		Shader* hullShader, Shader* domainShader) {

		this->rootSignature = rootSignature->Get();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		//シェーダ
		graphicsPipelineDesc.VS.pShaderBytecode = vertexShader->Get()->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = vertexShader->Get()->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = pixcelShader->Get()->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = pixcelShader->Get()->GetBufferSize();

		if (geometrtShader) {
			graphicsPipelineDesc.GS.pShaderBytecode = geometrtShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.GS.BytecodeLength = geometrtShader->Get()->GetBufferSize();
		}

		if (hullShader) {
			graphicsPipelineDesc.HS.pShaderBytecode = hullShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.HS.BytecodeLength = hullShader->Get()->GetBufferSize();
		}

		if (domainShader) {
			graphicsPipelineDesc.DS.pShaderBytecode = domainShader->Get()->GetBufferPointer();
			graphicsPipelineDesc.DS.BytecodeLength = domainShader->Get()->GetBufferSize();
		}

		//頂点シェーダへの入力情報のレイアウト
		
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs(vertexLayouts.size(), D3D12_INPUT_ELEMENT_DESC{});
		std::uint32_t alignedByteOffset = 0;
		for (std::size_t i = 0; i < vertexLayouts.size(); i++)
		{
			inputElementDescs[i].SemanticName = vertexLayouts[i].name;
			inputElementDescs[i].AlignedByteOffset = alignedByteOffset;
			inputElementDescs[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

			if (vertexLayouts[i].format == VertexLayoutFormat::Float2) {
				inputElementDescs[i].Format = DXGI_FORMAT_R32G32_FLOAT;
				alignedByteOffset += sizeof(float) * 2;
			}
			else if (vertexLayouts[i].format == VertexLayoutFormat::Float3) {
				inputElementDescs[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				alignedByteOffset += sizeof(float) * 3;
			}
			else if (vertexLayouts[i].format == VertexLayoutFormat::Float4) {
				inputElementDescs[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				alignedByteOffset += sizeof(float) * 4;
			}
		}

		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputElementDescs.data();
		graphicsPipelineDesc.InputLayout.NumElements = inputElementDescs.size();

		//とりあえず
		if (hullShader && domainShader)
			graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		else
			graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		//カット値はナシ
		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

		//ラスタライザの設定
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		//中身を塗りつぶす
		//rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
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
		renderTagetBlendDesc.BlendEnable = FALSE;//とりあえず
		renderTagetBlendDesc.LogicOpEnable = FALSE;
		renderTagetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = TRUE;
		blendDesc.IndependentBlendEnable = FALSE;
		//renderTagetBlendDesc.BlendEnableがFalseなので1つ目のみ設定で十分
		blendDesc.RenderTarget[0] = renderTagetBlendDesc;

		graphicsPipelineDesc.BlendState = blendDesc;


		graphicsPipelineDesc.NumRenderTargets = renderTargetFormats.size();
		for (std::size_t i = 0; i < renderTargetFormats.size(); i++) 
			graphicsPipelineDesc.RTVFormats[i] = static_cast<DXGI_FORMAT>(renderTargetFormats[i]);


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

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

		graphicsPipelineDesc.pRootSignature = rootSignature->Get();

		if (FAILED(device->Get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&pipelineState))))
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