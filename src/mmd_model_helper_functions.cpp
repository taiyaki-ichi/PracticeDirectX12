#include"mmd_model_helper_functions.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/shader.hpp"

namespace ichi
{

	std::optional<ID3D12RootSignature*> create_mmd_rootsignature(device* device)
	{
		ID3D12RootSignature* rootSignature = nullptr;

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
		//
		rootSignatureDesc.NumParameters = 3;//���[�g�p�����[�^��
		//

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

			return std::nullopt;
		}

		result = device->get()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
		if (FAILED(result)) {
			std::cout << "CreateRootSignature is falied : CreateRootSignature\n";
			return std::nullopt;
		}

		rootSigBlob->Release();

		return rootSignature;
	}

	std::optional<std::pair<ID3D12PipelineState*, ID3D12PipelineState*>> create_mmd_pipline_state(device* device, ID3D12RootSignature* rootSignature)
	{
		if (!rootSignature)
			return std::nullopt;


		auto vertShaderBlob = create_shader_blob(L"shader/VertexShader.hlsl", "main", "vs_5_0");
		auto pixcShaderBlob = create_shader_blob(L"shader/PixelShader.hlsl", "main", "ps_5_0");
		auto shadowVertShaderBolb = create_shader_blob(L"shader/VertexShader.hlsl", "shadowVS", "vs_5_0");

		auto releaseShaders = [&vertShaderBlob, &pixcShaderBlob, &shadowVertShaderBolb]() {
			if (vertShaderBlob)
				vertShaderBlob->Release();
			if (pixcShaderBlob)
				pixcShaderBlob->Release();
			if (shadowVertShaderBolb)
				shadowVertShaderBolb->Release();
		};

		//�܂��͕��ʂ̂����
		ID3D12PipelineState* pipelineState = nullptr;

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
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
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
		graphicsPipelineDesc.pRootSignature = rootSignature;

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&pipelineState))))
		{
			std::cout << "mmd CreateGraphicsPipelineState is failed\n";
			releaseShaders();
			return std::nullopt;
		}


		//���C�g�[�x�p�̃p�C�v���C���X�e�[�g
		ID3D12PipelineState* shadowPipelineState = nullptr;

		graphicsPipelineDesc.VS.pShaderBytecode = shadowVertShaderBolb->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = shadowVertShaderBolb->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = nullptr;
		graphicsPipelineDesc.PS.BytecodeLength = 0;

		//�����_�[�^�[�Q�b�g�K�v�Ȃ�
		graphicsPipelineDesc.NumRenderTargets = 0;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		//[1]�̃N���A��Y�ꂸ��
		graphicsPipelineDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
		graphicsPipelineDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&shadowPipelineState))))
		{
			std::cout << "mmd shadow CreateGraphicsPipelineState is failed\n";
			releaseShaders();
			if (pipelineState)
				pipelineState->Release();
			return std::nullopt;
		}

		releaseShaders();
		return std::make_pair(pipelineState, shadowPipelineState);
	}


	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>>
		create_mmd_depth_buffers(device* device, unsigned int windowWidth, unsigned int windowHeight, unsigned int shadowDifinition)
	{

		ID3D12Resource* depthBuffer = nullptr;

		//�[�x�o�b�t�@�̎d�l
		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2�����̃e�N�X�`���f�[�^�Ƃ���
		depthResDesc.Width = windowWidth;//���ƍ����̓����_�[�^�[�Q�b�g�Ɠ���
		depthResDesc.Height = windowHeight;//��ɓ���
		depthResDesc.DepthOrArraySize = 1;//�e�N�X�`���z��ł��Ȃ���3D�e�N�X�`���ł��Ȃ�
		depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;//�[�x�l�������ݗp�t�H�[�}�b�g
		depthResDesc.SampleDesc.Count = 1;//�T���v����1�s�N�Z��������1��
		depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//���̃o�b�t�@�͐[�x�X�e���V���Ƃ��Ďg�p���܂�
		depthResDesc.MipLevels = 1;
		depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthResDesc.Alignment = 0;

		//�f�v�X�p�q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES depthHeapProp{};
		depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULT��������UNKNOWN�ł悵
		depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//���̃N���A�o�����[���d�v�ȈӖ�������
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.0f;//�[���P(�ő�l)�ŃN���A
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit�[�x�l�Ƃ��ăN���A

		if (FAILED(device->get()->CreateCommittedResource(
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
			&depthClearValue,
			IID_PPV_ARGS(&depthBuffer)
		))) {
			std::cout << "mmd depth_buffer initialize is failed\n";
			return std::nullopt;
		}


		ID3D12Resource* lightDepthBuffer = nullptr;

		depthResDesc.Width = shadowDifinition;
		depthResDesc.Height = shadowDifinition;
		//depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;

		if (FAILED(device->get()->CreateCommittedResource(
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
			&depthClearValue,
			IID_PPV_ARGS(&lightDepthBuffer)
		))) {
			std::cout << "mmd light depth initialize is failed\n";
			if (depthBuffer)
				depthBuffer->Release();
			return std::nullopt;
		}


		return std::make_pair(depthBuffer, lightDepthBuffer);
	}
}