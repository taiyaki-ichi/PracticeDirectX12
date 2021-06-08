#pragma once
#include"../RootSignature/RootSignature.hpp"
#include"VertexLayout.hpp"
#include"RenderTarget.hpp"
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

	public:
		PipelineState(const  PipelineState&) = delete;
		PipelineState& operator=(const PipelineState&) = delete;

		PipelineState(PipelineState&&) noexcept;
		PipelineState& operator=(PipelineState&&) noexcept;

		void Initialize(Device*, RootSignature*, Shader* vertexShader, Shader* pixcelShader,
			const std::vector<VertexLayout>&, const std::vector<RenderTargetFormat>&, bool depthEnable);

		//preparefor

	};


	//
	//
	//

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
		const std::vector<RenderTargetFormat>& renderTargetFormats,bool depthEnable) {

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		//�V�F�[�_
		graphicsPipelineDesc.VS.pShaderBytecode = vertexShader->Get()->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = vertexShader->Get()->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = pixcelShader->Get()->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = pixcelShader->Get()->GetBufferSize();

		//���_�V�F�[�_�ւ̓��͏��̃��C�A�E�g
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

		//�g�|���W�͎O�p�`�Œ�
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		//�J�b�g�l�̓i�V
		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;


		//���X�^���C�U�̐ݒ�
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		//���g��h��Ԃ�
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		//�J�����O�͂��Ȃ��B�܂�ʂ̌��������ł��`�ʂ���
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;//�V���h�E�}�b�v����Ƃ��͕ύX������
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.ForcedSampleCount = 0;
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		graphicsPipelineDesc.RasterizerState = rasterizerDesc;


		//�u�����h�X�e�[�g
		D3D12_RENDER_TARGET_BLEND_DESC renderTagetBlendDesc{};
		renderTagetBlendDesc.BlendEnable = FALSE;//�Ƃ肠����
		renderTagetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		//renderTagetBlendDesc.BlendEnable��False�Ȃ̂�1�ڂ̂ݐݒ�ŏ\��
		blendDesc.RenderTarget[0] = renderTagetBlendDesc;

		graphicsPipelineDesc.BlendState = blendDesc;


		graphicsPipelineDesc.NumRenderTargets = renderTargetFormats.size();
		//�������Ȃ��̂łƂ肠�����̃R�[�h
		for (std::size_t i = 0; i < renderTargetFormats.size(); i++) {
			if (renderTargetFormats[i] == RenderTargetFormat::R8G8B8A8)
				graphicsPipelineDesc.RTVFormats[i] = DXGI_FORMAT_B8G8R8X8_UNORM;
			else if (renderTargetFormats[i] == RenderTargetFormat::R32_FLOAT)
				graphicsPipelineDesc.RTVFormats[i] = DXGI_FORMAT_R32_FLOAT;
		}


		//�f�v�X�X�e���V��
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = depthEnable;
		if (depthEnable) {
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			graphicsPipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		}
		depthStencilDesc.StencilEnable = FALSE;

		graphicsPipelineDesc.DepthStencilState = depthStencilDesc;

		//�T���v���f�B�X�N
		graphicsPipelineDesc.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
		graphicsPipelineDesc.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

		if (FALSE(device->Get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&pipelineState))))
			throw "";
	}

}