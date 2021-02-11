#include"perapolygon_helper_functions.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/shader.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"window_size.hpp"
#include"DirectX12/resource_helper_functions.hpp"

namespace ichi
{
	std::optional<ID3D12RootSignature*> create_perapolygon_root_signature(device* device)
	{
		ID3D12RootSignature* resultPtr = nullptr;

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//�e�N�X�`��
		D3D12_DESCRIPTOR_RANGE range{};
		range.NumDescriptors = 6;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.BaseShaderRegister = 0;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		D3D12_ROOT_PARAMETER rootparam{};
		rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootparam.DescriptorTable.pDescriptorRanges = &range;
		rootparam.DescriptorTable.NumDescriptorRanges = 1;

		rootSignatureDesc.NumParameters = 1;
		rootSignatureDesc.pParameters = &rootparam;

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

		rootSignatureDesc.NumStaticSamplers = 1;
		rootSignatureDesc.pStaticSamplers = &samplerDesc;

		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		if (FAILED(result)) {
			std::cout << "perapolygon D3D12SerializeRootSignature is failed : D3D12SerializeRootSignature ";

			//�G���[���e
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			std::cout << " : " << errstr << "\n";

			return std::nullopt;
		}

		result = device->get()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&resultPtr));
		if (FAILED(result)) {
			std::cout << "perapolygon CreateRootSignature is falied : CreateRootSignature\n";
			return std::nullopt;
		}

		rootSigBlob->Release();

		return resultPtr;
	}


	std::optional<std::pair<ID3D12PipelineState*, ID3D12PipelineState*>> create_perapolygon_pipline_state(device* device, ID3D12RootSignature* rootSignature)
	{
		ID3D12PipelineState* result = nullptr;
		ID3D12PipelineState* result2 = nullptr;

		auto vertexShader = create_shader_blob(L"shader/peraVertexShader.hlsl", "main", "vs_5_0");
		auto pixelShader = create_shader_blob(L"shader/peraPixelShader.hlsl", "main", "ps_5_0");
		auto blurPixelShader = create_shader_blob(L"shader/peraPixelShader.hlsl", "BlurPS", "ps_5_0");


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

		//
		//���_���̃��C�A�E�g
		//�؂�|���S���p�ɃJ�X�^��
		//
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		};


		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
		graphicsPipelineDesc.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��

		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

		graphicsPipelineDesc.NumRenderTargets = 1;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0�`1�ɐ��K�����ꂽRGBA

		graphicsPipelineDesc.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
		graphicsPipelineDesc.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

		//���[�g�V�O�l�`��
		graphicsPipelineDesc.pRootSignature = rootSignature;

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&result))))
		{
			std::cout << "pera CreateGraphicsPipelineState is failed\n";
			vertexShader->Release();
			pixelShader->Release();
			blurPixelShader->Release();
			return std::nullopt;
		}

		//�s�N�Z���V�F�[�_�̐ݒ�
		graphicsPipelineDesc.PS.pShaderBytecode = blurPixelShader->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = blurPixelShader->GetBufferSize();

		graphicsPipelineDesc.DepthStencilState.DepthEnable = false;
		
		graphicsPipelineDesc.NumRenderTargets = 2;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		graphicsPipelineDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;


		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&result2))))
		{
			std::cout << "pera CreateGraphicsPipelineState 2 is failed\n";
			vertexShader->Release();
			pixelShader->Release();
			blurPixelShader->Release();
			result->Release();
			return std::nullopt;
		}

		vertexShader->Release();
		pixelShader->Release();
		blurPixelShader->Release();

		return std::make_pair(result, result2);
	}

	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>> create_perapolygon_resource(device* device)
	{
		ID3D12Resource* colorResource = nullptr;
		ID3D12Resource* normalResource = nullptr;

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		//resdesc.Alignment = 65536;
		resdesc.DepthOrArraySize = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		resdesc.MipLevels = 1;
		resdesc.Height = window_height;
		resdesc.Width = window_width;
		resdesc.SampleDesc = { 1,0 };
		resdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		//�����œn���l��Clear�̒l���قȂ�ƌx���o��
		//�x���Ȃ��A�݂�����
		D3D12_CLEAR_VALUE clearValue{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.5f,0.5f,0.5f,1.f } };

		//���ʂ�
		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,//rendertarget�ł͂Ȃ�
			&clearValue,
			IID_PPV_ARGS(&colorResource)
		))) {
			std::cout << "colorResource failed perapolygon render init \n";
			return std::nullopt;
		}

		//�@���p
		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,//rendertarget�ł͂Ȃ�
			&clearValue,
			IID_PPV_ARGS(&normalResource)
		))) {
			std::cout << "normalResource failed perapolygon render normal init \n";
			if (colorResource)
				colorResource->Release();
			return std::nullopt;
		}

	
		return std::make_pair(colorResource, normalResource);
	}


	std::optional<std::unique_ptr<vertex_buffer>> create_perapolygon_vertex_buffer(device* device)
	{
		auto peraVertexBuff = std::make_unique<ichi::vertex_buffer>();
		struct PeraVert {
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 uv;
		};
		PeraVert pv[4] = {
			{{-1.f,-1.f,0.1f},{0,1.f}},
			{{-1.f,1.f,0.1f},{0,0}},
			{{1.f,-1.f,0.1f},{1.f,1.f}},
			{{1.f,1.f,0.1f},{1.f,0}},
		};

		if (!peraVertexBuff->initialize(device, sizeof(pv), sizeof(pv[0]))) 
			return std::nullopt;
	
		map_to_resource(peraVertexBuff.get(), pv);

		return peraVertexBuff;
	}


	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>> create_perapolygon_bloom_resource(device* device)
	{
		ID3D12Resource* bloomResource = nullptr;
		ID3D12Resource* shrinkResource = nullptr;

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		//resdesc.Alignment = 65536;
		resdesc.DepthOrArraySize = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		resdesc.MipLevels = 1;
		resdesc.Height = window_height;
		resdesc.Width = window_width;
		resdesc.SampleDesc = { 1,0 };
		resdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		//�����œn���l��Clear�̒l���قȂ�ƌx���o��
		//�x���Ȃ��A�݂�����
		//�K�E�X�␳��������̂ŃN���A�J���[�͐^������
		D3D12_CLEAR_VALUE clearValue{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.f,0.f,0.f,1.f } };

		//bloom�p
		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,//rendertarget�ł͂Ȃ�
			&clearValue,
			IID_PPV_ARGS(&bloomResource)
		))) {
			std::cout << "bloomResource failed perapolygon render init \n";
			return std::nullopt;
		}

		//�����𔼕��ɏk��
		resdesc.Width >>= 1;

		//�k�����ꂽ���\�[�X
		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,//rendertarget�ł͂Ȃ�
			&clearValue,
			IID_PPV_ARGS(&shrinkResource)
		))) {
			std::cout << "shrinkBloomResource failed perapolygon render init \n";
			if (bloomResource)
				bloomResource->Release();
			return std::nullopt;
		}

		return std::make_pair(bloomResource, shrinkResource);
	}

	std::optional<ID3D12Resource*> create_perapolygon_DOF_resource(device* device)
	{
		ID3D12Resource* DOFResource = nullptr;

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resdesc.Alignment = 65536;
		resdesc.DepthOrArraySize = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		resdesc.MipLevels = 1;
		resdesc.Height = window_height;
		resdesc.Width = window_width / 2;//�k���o�b�t�@�Ȃ̂ł́[��Ԃ�
		resdesc.SampleDesc = { 1,0 };
		resdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		//�����œn���l��Clear�̒l���قȂ�ƌx���o��
		//�x���Ȃ��A�݂�����
		//�Ƃ肠������
		D3D12_CLEAR_VALUE clearValue{ DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.f,0.f,0.f,1.f } };

		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,//rendertarget�ł͂Ȃ�
			&clearValue,
			IID_PPV_ARGS(&DOFResource)
		))) {
			std::cout << "DOF Resource failed perapolygon render init \n";
			return std::nullopt;
		}

		return DOFResource;
	}
}