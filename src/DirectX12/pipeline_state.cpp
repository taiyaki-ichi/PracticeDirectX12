#include"pipeline_state.hpp"
#include"device.hpp"
#include"descriptor_heap.hpp"

#include<iostream>

namespace ichi
{
	pipeline_state::~pipeline_state()
	{
		if (m_pipeline_state)
			m_pipeline_state->Release();
		if (m_root_signature)
			m_root_signature->Release();
	}

	bool pipeline_state::initialize(device* device, ID3DBlob* vertexShader, ID3DBlob* pixelShader)
	{

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//�萔�ƃe�N�X�`��
		D3D12_DESCRIPTOR_RANGE range[2]{ {},{} };
		//Constant
		//���݂�2��
		//NumDescriptors�͗]�T�����镪�ɂ̓G���[���o�Ȃ�
		range[0].NumDescriptors = 3;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range[0].BaseShaderRegister = 0;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		//shader resource
		//a
		range[1].NumDescriptors =  1;
		range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[1].BaseShaderRegister = 0;
		range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER rootparam{};
		rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam.DescriptorTable.pDescriptorRanges = &range[0];//�f�X�N���v�^�����W�̃A�h���X
		rootparam.DescriptorTable.NumDescriptorRanges = 2;//�f�X�N���v�^�����W��
		rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�S�ẴV�F�[�_���猩����

		rootSignatureDesc.pParameters = &rootparam;//���[�g�p�����[�^�̐擪�A�h���X
		rootSignatureDesc.NumParameters = 1;//���[�g�p�����[�^��

		D3D12_STATIC_SAMPLER_DESC samplerDesc{};
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���J��Ԃ�
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�c�J��Ԃ�
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���s�J��Ԃ�
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�̎��͍�
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//��Ԃ��Ȃ�(�j�A���X�g�l�C�o�[)
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
		samplerDesc.MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//�I�[�o�[�T���v�����O�̍ۃ��T���v�����O���Ȃ��H
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����̂݉�

		rootSignatureDesc.pStaticSamplers = &samplerDesc;
		rootSignatureDesc.NumStaticSamplers = 1;

		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		if (FAILED(result)) {
			std::cout << "D3D12SerializeRootSignature is failed : D3D12SerializeRootSignature ";

			//�G���[���e
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			std::cout << " : " << errstr << "\n";

			return false;
		}

		result = device->get()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature));
		if (FAILED(result)) {
			std::cout << "CreateRootSignature is falied : CreateRootSignature\n";
			return false;
		}

		rootSigBlob->Release();

		
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		graphicsPipelineDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = pixelShader->GetBufferSize();

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//���g��0xffffffff

		graphicsPipelineDesc.BlendState.AlphaToCoverageEnable = false;
		graphicsPipelineDesc.BlendState.IndependentBlendEnable = false;


		//���X�^���C�U�[�X�e�[�g�ݒ�p�\���̂̐ݒ�
		D3D12_RENDER_TARGET_BLEND_DESC renderBlend{};
		renderBlend.BlendEnable = FALSE;
		renderBlend.BlendOp = D3D12_BLEND_OP_ADD;
		renderBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderBlend.DestBlend = D3D12_BLEND_ZERO;
		renderBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
		renderBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
		renderBlend.LogicOpEnable = FALSE;
		renderBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		renderBlend.SrcBlend = D3D12_BLEND_ONE;
		renderBlend.SrcBlendAlpha = D3D12_BLEND_ONE;

		//�u�����h�X�e�[�g�ݒ�p�\����
		D3D12_BLEND_DESC blend{};
		blend.AlphaToCoverageEnable = TRUE;
		blend.IndependentBlendEnable = FALSE;
		//�Ƃ肠����0�����ɂ��Ă���
		blend.RenderTarget[0] = renderBlend;

		graphicsPipelineDesc.BlendState = blend;

		graphicsPipelineDesc.RasterizerState.MultisampleEnable = false;//�܂��A���`�F���͎g��Ȃ�
		graphicsPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�
		graphicsPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���g��h��Ԃ�
		graphicsPipelineDesc.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�͗L����

		//�c��
		graphicsPipelineDesc.RasterizerState.FrontCounterClockwise = false;
		graphicsPipelineDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		graphicsPipelineDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.AntialiasedLineEnable = false;
		graphicsPipelineDesc.RasterizerState.ForcedSampleCount = 0;
		graphicsPipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		graphicsPipelineDesc.DepthStencilState.DepthEnable = true;
		graphicsPipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		graphicsPipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		graphicsPipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		graphicsPipelineDesc.DepthStencilState.StencilEnable = false;

		//���_���̃��C�A�E�g
		//D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			//{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			//{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		//};
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		};


		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
		graphicsPipelineDesc.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��

		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

		graphicsPipelineDesc.NumRenderTargets = 1;//���͂P�̂�
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0�`1�ɐ��K�����ꂽRGBA

		graphicsPipelineDesc.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
		graphicsPipelineDesc.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

		//���[�g�V�O�l�`��
		graphicsPipelineDesc.pRootSignature = m_root_signature;

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_pipeline_state))))
		{
			std::cout << "CreateGraphicsPipelineState is failed\n";
			return false;
		}

		return true;
	}
	ID3D12PipelineState* pipeline_state::get() const noexcept
	{
		return m_pipeline_state;
	}
	ID3D12RootSignature* pipeline_state::get_root_signature() const noexcept
	{
		return m_root_signature;
	}
}