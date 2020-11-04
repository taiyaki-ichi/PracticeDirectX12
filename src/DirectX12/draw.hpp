#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<iostream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace graphics
{

	//���[�g�V�O�l�`���̐���
	ID3D12RootSignature* create_root_signature(ID3D12Device* device)
	{
		ID3D12RootSignature* rootSignature = nullptr;

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		D3D12_DESCRIPTOR_RANGE descTblRange{};
		descTblRange.NumDescriptors = 1;//�e�N�X�`���ЂƂ�
		descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//��ʂ̓e�N�X�`��
		descTblRange.BaseShaderRegister = 0;//0�ԃX���b�g����
		descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		D3D12_ROOT_PARAMETER rootparam{};
		rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;//�f�X�N���v�^�����W�̃A�h���X
		rootparam.DescriptorTable.NumDescriptorRanges = 1;//�f�X�N���v�^�����W��
		rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_���猩����

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
			std::cout << __func__ << " is failed : D3D12SerializeRootSignature ";

			//�G���[���e
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			std::cout << " : " << errstr << "\n";

			return nullptr;
		}

		result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
		if (FAILED(result)) {
			std::cout << __func__ << " is falied : CreateRootSignature\n";
			return nullptr;
		}

		rootSigBlob->Release();

		return rootSignature;
	}

	//�O���t�B�b�N�X�p�C�v���C���̐���
	ID3D12PipelineState* create_graphics_pipline_state(ID3D12Device* device,ID3DBlob* vertexShader, ID3DBlob* pixelShader, ID3D12RootSignature* rootSignature)
	{

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};
		graphicsPipelineDesc.pRootSignature = nullptr;
		graphicsPipelineDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = pixelShader->GetBufferSize();

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//���g��0xffffffff

		//
		graphicsPipelineDesc.BlendState.AlphaToCoverageEnable = false;
		graphicsPipelineDesc.BlendState.IndependentBlendEnable = false;

		D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc{};

		//�ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
		renderTargetBlendDesc.BlendEnable = false;
		renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		//�ЂƂ܂��_�����Z�͎g�p���Ȃ�
		renderTargetBlendDesc.LogicOpEnable = false;

		graphicsPipelineDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;


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


		graphicsPipelineDesc.DepthStencilState.DepthEnable = false;
		graphicsPipelineDesc.DepthStencilState.StencilEnable = false;

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
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
		graphicsPipelineDesc.pRootSignature = rootSignature;

		ID3D12PipelineState* graphicsPipline = nullptr;
		if (FAILED(device->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&graphicsPipline))))
		{
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}

		return graphicsPipline;
	}

	//�r���[�|�[�g�̎擾
	D3D12_VIEWPORT get_viewport(unsigned int width, unsigned int height)
	{
		D3D12_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(width);//�o�͐�̕�(�s�N�Z����)
		viewport.Height = static_cast<float>(height);//�o�͐�̍���(�s�N�Z����)
		viewport.TopLeftX = 0;//�o�͐�̍�����WX
		viewport.TopLeftY = 0;//�o�͐�̍�����WY
		viewport.MaxDepth = 1.0f;//�[�x�ő�l
		viewport.MinDepth = 0.0f;//�[�x�ŏ��l

		return viewport;
	}

	//�V�U�[��`�̎擾
	D3D12_RECT get_scissor_rect(unsigned int width, unsigned int height)
	{
		D3D12_RECT scissorrect{};
		scissorrect.top = 0;//�؂蔲������W
		scissorrect.left = 0;//�؂蔲�������W
		scissorrect.right = scissorrect.left + width;//�؂蔲���E���W
		scissorrect.bottom = scissorrect.top + height;//�؂蔲�������W

		return scissorrect;
	}

}