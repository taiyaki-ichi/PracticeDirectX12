#include"mmd_model_renderer.hpp"
#include"DirectX12/shader.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/command_list.hpp"

namespace DX12
{

	mmd_model_renderer::~mmd_model_renderer()
	{
		if (m_pipeline_state)
			m_pipeline_state->Release();
		if (m_root_signature)
			m_root_signature->Release();
		if (m_light_depth_pipeline_state)
			m_light_depth_pipeline_state->Release();
	}

	bool mmd_model_renderer::initialize(device* device)
	{

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//���[�g�V�O�l�`���̍쐻
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		D3D12_DESCRIPTOR_RANGE range[]{ {},{} ,{},{} };
		//���_���
		range[0].NumDescriptors = 1;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range[0].BaseShaderRegister = 0;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//�}�e���A��̊�{�f�[�^
		range[1].NumDescriptors = 1;
		range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range[1].BaseShaderRegister = 1;
		range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//�}�e���A���̃X�t�B�A�Ƃ��g�D�[��
		range[2].NumDescriptors = 4;
		range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[2].BaseShaderRegister = 0;
		range[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//���C�g�̐[�x�p
		range[3].NumDescriptors = 1;
		range[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[3].BaseShaderRegister = 4;
		range[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		D3D12_ROOT_PARAMETER rootparam[]{ {},{},{} };
		rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[0].DescriptorTable.pDescriptorRanges = &range[0];//�f�X�N���v�^�����W�̃A�h���X
		rootparam[0].DescriptorTable.NumDescriptorRanges = 1;//�f�X�N���v�^�����W��
		rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�S�ẴV�F�[�_���猩����

		//�}�e���A�����Ƃ�GPU�n���h�����ꏏ�ɂ��炷���߂܂Ƃ߂�
		rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[1].DescriptorTable.pDescriptorRanges = &range[1];//�f�X�N���v�^�����W�̃A�h���X
		rootparam[1].DescriptorTable.NumDescriptorRanges = 2;//�f�X�N���v�^�����W��
		rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�S�ẴV�F�[�_���猩����

		rootparam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[2].DescriptorTable.pDescriptorRanges = &range[3];//�f�X�N���v�^�����W�̃A�h���X
		rootparam[2].DescriptorTable.NumDescriptorRanges = 1;//�f�X�N���v�^�����W��
		rootparam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�S�ẴV�F�[�_���猩����

		//�܂Ƃ߂�
		rootSignatureDesc.pParameters = rootparam;//���[�g�p�����[�^�̐擪�A�h���X
		rootSignatureDesc.NumParameters = 3;//���[�g�p�����[�^��

		//�ʏ�̂ƁA�g�D�[���p
		D3D12_STATIC_SAMPLER_DESC samplerDesc[]{ {},{} ,{} };
		samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���J��Ԃ�
		samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�c�J��Ԃ�
		samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���s�J��Ԃ�
		samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�̎��͍�
		samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//��Ԃ��Ȃ�(�j�A���X�g�l�C�o�[)
		samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
		samplerDesc[0].MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
		samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//�I�[�o�[�T���v�����O�̍ۃ��T���v�����O���Ȃ��H
		samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����̂݉�

		samplerDesc[1] = samplerDesc[0];//�ύX�_�ȊO���R�s�[
		samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//
		samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[1].ShaderRegister = 1;

		samplerDesc[2] = samplerDesc[1];
		samplerDesc[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplerDesc[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;//��r���ʂ����j�A�⊮
		samplerDesc[2].MaxAnisotropy = 1;//�[�x�X�΂�L����
		samplerDesc[2].ShaderRegister = 2;

		//�܂Ƃ߂�
		rootSignatureDesc.pStaticSamplers = samplerDesc;
		rootSignatureDesc.NumStaticSamplers = 3;


		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		if (FAILED(result)) {
			std::cout << "mmd D3D12SerializeRootSignature is failed ";

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
		}

		rootSigBlob->Release();



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//�O���t�B�b�N�X�p�C�v���C���̍쐻
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		auto vertShaderBlob = create_shader_blob(L"shader/VertexShader.hlsl", "main", "vs_5_0");
		auto pixcShaderBlob = create_shader_blob(L"shader/PixelShader.hlsl", "main", "ps_5_0");
		auto shadowVertShaderBlob = create_shader_blob(L"shader/VertexShader.hlsl", "shadowVS", "vs_5_0");


		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		graphicsPipelineDesc.VS.pShaderBytecode = vertShaderBlob->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = vertShaderBlob->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = pixcShaderBlob->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = pixcShaderBlob->GetBufferSize();

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//���g��0xffffffff

		graphicsPipelineDesc.BlendState.AlphaToCoverageEnable = false;
		graphicsPipelineDesc.BlendState.IndependentBlendEnable = false;


		//���X�^���C�U�[�X�e�[�g�ݒ�p�\���̂̐ݒ�
		D3D12_RENDER_TARGET_BLEND_DESC renderBlend{};
		renderBlend.BlendEnable = TRUE;// FALSE;
		renderBlend.BlendOp = D3D12_BLEND_OP_ADD;
		renderBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderBlend.DestBlend = D3D12_BLEND_ZERO;
		renderBlend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA; //D3D12_BLEND_ZERO;
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
		graphicsPipelineDesc.RasterizerState.DepthBias = 0;// D3D12_DEFAULT_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		graphicsPipelineDesc.RasterizerState.SlopeScaledDepthBias = 0.01f;// D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.AntialiasedLineEnable = false;
		graphicsPipelineDesc.RasterizerState.ForcedSampleCount = 0;
		graphicsPipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		graphicsPipelineDesc.DepthStencilState.DepthEnable = true;
		graphicsPipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		graphicsPipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		graphicsPipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		graphicsPipelineDesc.DepthStencilState.StencilEnable = false;

		//���_���̃��C�A�E�g
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },

			{ "BONE_TYPE_FLAG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			//unsigned short[4]
			{ "BONENO",0,DXGI_FORMAT_R16G16B16A16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			//float[4]...32���Ⴀ�Ȃ���16�ł��悳����������
			{ "WEIGHT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			//SDEF�p
			//�z��ɂ�������Ƃ��A���̂���
			{ "SDEF_ONE",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{ "SDEF_TWO",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{ "SDEF_TREE",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		};


		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
		graphicsPipelineDesc.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��

		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

		graphicsPipelineDesc.NumRenderTargets = 3;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0�`1�ɐ��K�����ꂽRGBA
		graphicsPipelineDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
		graphicsPipelineDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;

		graphicsPipelineDesc.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
		graphicsPipelineDesc.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

		//���[�g�V�O�l�`��
		graphicsPipelineDesc.pRootSignature = m_root_signature;

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_pipeline_state))))
		{
			std::cout << "mmd CreateGraphicsPipelineState is failed\n";
		}


		//���C�g�[�x�p�̃p�C�v���C���X�e�[�g
		ID3D12PipelineState* shadowPipelineState = nullptr;

		graphicsPipelineDesc.VS.pShaderBytecode = shadowVertShaderBlob->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = shadowVertShaderBlob->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = nullptr;
		graphicsPipelineDesc.PS.BytecodeLength = 0;

		//�����_�[�^�[�Q�b�g�K�v�Ȃ�
		graphicsPipelineDesc.NumRenderTargets = 0;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		//[1]�̃N���A��Y�ꂸ��
		graphicsPipelineDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
		graphicsPipelineDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_light_depth_pipeline_state))))
		{
			std::cout << "mmd shadow CreateGraphicsPipelineState is failed\n";
		}



		vertShaderBlob->Release();
		pixcShaderBlob->Release();
		shadowVertShaderBlob->Release();


		if (m_root_signature && m_pipeline_state && m_light_depth_pipeline_state)
			return true;
		else
			return false;
	}

	void mmd_model_renderer::preparation_for_drawing(command_list* cl)
	{
		cl->get()->SetPipelineState(m_pipeline_state);
		cl->get()->SetGraphicsRootSignature(m_root_signature);

		cl->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void mmd_model_renderer::preparation_for_drawing_light_depth(command_list* cl)
	{
		cl->get()->SetPipelineState(m_light_depth_pipeline_state);
		cl->get()->SetGraphicsRootSignature(m_root_signature);

		cl->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	mmd_model_renderer::mmd_model_renderer(mmd_model_renderer&& r) noexcept
	{
		m_root_signature = r.m_root_signature;
		m_pipeline_state = r.m_pipeline_state;
		m_light_depth_pipeline_state = r.m_light_depth_pipeline_state;
		r.m_root_signature = nullptr;
		r.m_pipeline_state = nullptr;
		r.m_light_depth_pipeline_state = nullptr;
	}

	mmd_model_renderer& mmd_model_renderer::operator=(mmd_model_renderer&& r) noexcept
	{
		m_root_signature = r.m_root_signature;
		m_pipeline_state = r.m_pipeline_state;
		m_light_depth_pipeline_state = r.m_light_depth_pipeline_state;
		r.m_root_signature = nullptr;
		r.m_pipeline_state = nullptr;
		r.m_light_depth_pipeline_state = nullptr;
		return *this;
	}




}