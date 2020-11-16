#include"rect.hpp"
#include<Windows.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#include<iostream>
#include"descriptor_heap.hpp"
#include<array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace graphics
{
	rect::rect(ID3D12Device* device,float window_width, float window_height)
		: m_matrix{}
		, m_descriptor_heap{nullptr}
		, m_root_signature{nullptr}
		, m_vert_shader_blob{nullptr}
		, m_pixcel_shader_blob{ nullptr }
		, m_vertex_buffer{nullptr}
		, m_index_buffer{nullptr}
		, m_constant_matrix_buffer{nullptr}
		, m_constant_color_buffer{nullptr}
		, m_vertex_view{}
		, m_index_view{}
		, m_graphics_pipeline_state{nullptr}
	{

		
		m_matrix.r[0].m128_f32[0] = 2.f / window_width;
		m_matrix.r[1].m128_f32[1] = 2.f / window_height;

		m_matrix.r[3].m128_f32[0] = -1.f;
		m_matrix.r[3].m128_f32[1] = 1.f;

		//���\�[�X�̊m��
		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resdesc.Height = 1;
		resdesc.DepthOrArraySize = 1;
		resdesc.MipLevels = 1;
		resdesc.Format = DXGI_FORMAT_UNKNOWN;
		resdesc.SampleDesc.Count = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		DirectX::XMFLOAT3 v[] = {
			{-0.5f,-0.5f,0.0f},//����
			{-0.5f,0.5f,0.0f} ,//����
			{0.5f,-0.5f,0.0f} ,//�E��
			{0.5f,0.5f,0.0f} ,//�E��
		};


		//���_
		resdesc.Width = sizeof(v);
		device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertex_buffer));

		m_vertex_buffer->Map(0, nullptr, (void**)&v);


		//�C���f�b�N�X
		unsigned int i[] = { 0,1,2, 2,1,3 };
		//���\�[�X
		resdesc.Width = sizeof(i);
		device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_index_buffer));

		m_index_buffer->Map(0, nullptr, (void**)&i);

		//�s��
		//�A���C�����g�Y����
		resdesc.Width = (sizeof(m_matrix) + 0xff) & ~0xff;
		device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constant_matrix_buffer));


		m_constant_matrix_buffer->Map(0, nullptr, (void**)&m_matrix);

		//�F
		float c[] = { 0,1,1,1 };
		//�A���C�����g�Y����
		resdesc.Width = (sizeof(c) + 0xff) & ~0xff;
		device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constant_color_buffer));

		m_constant_matrix_buffer->Map(0, nullptr, (void**)&c);
		

		//�r���[
		m_vertex_view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
		m_vertex_view.SizeInBytes = sizeof(v);
		m_vertex_view.StrideInBytes = sizeof(v[0]);
		//�r���[
		m_index_view.BufferLocation = m_index_buffer->GetGPUVirtualAddress();
		m_index_view.SizeInBytes = sizeof(i);
		m_index_view.Format = DXGI_FORMAT_R16_UINT;
		
		//�V�F�[�_�I�u�W�F�N�g�̐���
		ID3DBlob* errorBlob = nullptr;
		auto errorFunc = [](HRESULT r,const char* funcName, ID3DBlob* e) {
			std::cout << funcName << " is failed ";
			if (r == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
				std::cout << " : file is not found\n";
			}
			else {
				std::string errstr;
				errstr.resize(e->GetBufferSize());
				std::copy_n((char*)e->GetBufferPointer(), e->GetBufferSize(), errstr.begin());
				std::cout << " : " << errstr << "\n";
			}
		};
		//���_
		HRESULT result;
		result = D3DCompileFromFile(
			L"shader/RectVertexShader.hlsl",
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, &m_vert_shader_blob, &errorBlob);
		//���s�����ꍇ
		if (FAILED(result))
			errorFunc(result, __func__, errorBlob);

		//�s�N�Z��
		result = D3DCompileFromFile(
			L"shader/RectPixcelShader.hlsl",
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "ps_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, &m_pixcel_shader_blob, &errorBlob);
		//���s�����ꍇ
		if (FAILED(result))
			errorFunc(result, __func__, errorBlob);

		
		//�s��ƐF
		D3D12_DESCRIPTOR_RANGE discriptorRangeDesc[2]{};
		//�s��
		discriptorRangeDesc[0].NumDescriptors = 1;
		discriptorRangeDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		discriptorRangeDesc[0].BaseShaderRegister = 0;
		discriptorRangeDesc[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		//�F
		discriptorRangeDesc[1].NumDescriptors = 1;
		discriptorRangeDesc[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		discriptorRangeDesc[1].BaseShaderRegister = 1;
		discriptorRangeDesc[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		//���[�g�p����(�f�B�X�N���v�^�e�[�u��)
		D3D12_ROOT_PARAMETER rootParam[2]{};
		//�s��
		rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[0].DescriptorTable.pDescriptorRanges = &discriptorRangeDesc[0];
		rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		//�F
		rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[1].DescriptorTable.pDescriptorRanges = &discriptorRangeDesc[1];
		rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


		//���[�g�V�O�l�`��
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		rootSignatureDesc.pParameters = &rootParam[0];
		rootSignatureDesc.NumParameters = 2;
		rootSignatureDesc.NumStaticSamplers = 0;

		ID3DBlob* root_signature_blob = nullptr;
		result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,&root_signature_blob, &errorBlob);
		if (FAILED(result)) {
			std::cout << __func__ << " is failed : D3D12SerializeRootSignature ";
			//�G���[���e
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			std::cout << " : " << errstr << "\n";
		}
		result = device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature));
		if (FAILED(result)) {
			std::cout << __func__ << " is falied : CreateRootSignature\n";
		}
		root_signature_blob->Release();


		//���X�^���C�U�[
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
		blend.RenderTarget[0] = renderBlend;

		//���_���̃��C�A�E�g
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 }
		};

		//�p�C�v���C��
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};
		graphicsPipelineDesc.VS.pShaderBytecode = m_vert_shader_blob->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = m_vert_shader_blob->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = m_pixcel_shader_blob->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = m_pixcel_shader_blob->GetBufferSize();
		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//���g��0xffffffff
		graphicsPipelineDesc.BlendState.AlphaToCoverageEnable = false;
		graphicsPipelineDesc.BlendState.IndependentBlendEnable = false;
		graphicsPipelineDesc.BlendState = blend;
		graphicsPipelineDesc.RasterizerState.MultisampleEnable = false;//�܂��A���`�F���͎g��Ȃ�
		graphicsPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�
		graphicsPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���g��h��Ԃ�
		graphicsPipelineDesc.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�͗L����
		graphicsPipelineDesc.RasterizerState.FrontCounterClockwise = false;
		graphicsPipelineDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		graphicsPipelineDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.AntialiasedLineEnable = false;
		graphicsPipelineDesc.RasterizerState.ForcedSampleCount = 0;
		graphicsPipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		graphicsPipelineDesc.DepthStencilState.DepthEnable = false;
		graphicsPipelineDesc.DepthStencilState.StencilEnable = false;
		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
		graphicsPipelineDesc.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��
		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��
		graphicsPipelineDesc.NumRenderTargets = 1;//���͂P�̂�
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0�`1�ɐ��K�����ꂽRGBA
		graphicsPipelineDesc.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
		graphicsPipelineDesc.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�
		graphicsPipelineDesc.pRootSignature = m_root_signature;

		

		result = device->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_graphics_pipeline_state));
		if (FAILED(result))
			std::cout << __func__ << " is failed : CreateGraphicsPipelineState : "<<result<<" \n";


		//�f�B�X�N���v�^�q�[�v
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descHeapDesc.NodeMask = 0;
		//�}�g���b�N�X�A�F�̎O��
		descHeapDesc.NumDescriptors = 2;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//����
		device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_descriptor_heap));

		//�f�X�N���v�^�̐擪�n���h��
		auto basicHeapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		
		//�萔
		D3D12_CONSTANT_BUFFER_VIEW_DESC constantMatrixDesc{};
		constantMatrixDesc.BufferLocation = m_constant_matrix_buffer->GetGPUVirtualAddress();
		constantMatrixDesc.SizeInBytes = static_cast<UINT>(m_constant_matrix_buffer->GetDesc().Width);
		//�萔�o�b�t�@�r���[�̍쐬
		device->CreateConstantBufferView(&constantMatrixDesc, basicHeapHandle);

		basicHeapHandle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//�F
		D3D12_CONSTANT_BUFFER_VIEW_DESC constantColorDesc{};
		constantColorDesc.BufferLocation = m_constant_color_buffer->GetGPUVirtualAddress();
		constantColorDesc.SizeInBytes = static_cast<UINT>(m_constant_color_buffer->GetDesc().Width);
		//�萔�o�b�t�@�r���[�̍쐬
		device->CreateConstantBufferView(&constantColorDesc, basicHeapHandle);
		
		

	}
	void rect::draw_command(ID3D12GraphicsCommandList* cml)
	{

		cml->SetPipelineState(m_graphics_pipeline_state);
		cml->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cml->IASetVertexBuffers(0, 1, &m_vertex_view);
		cml->IASetIndexBuffer(&m_index_view);
		cml->SetGraphicsRootSignature(m_root_signature);
		cml->SetDescriptorHeaps(1, &m_descriptor_heap);
		cml->SetGraphicsRootDescriptorTable(0, m_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
		cml->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	void rect::set_RGBA(float r, float g, float b, float a)
	{
		float c[] = { r,g,b,a };
		m_constant_matrix_buffer->Map(0, nullptr, (void**)&c);
	}
	void rect::set_width_and_height(float, float)
	{
		//
		//
		//
		m_constant_matrix_buffer->Map(0, nullptr, (void**)&m_matrix);
	}
	void rect::set_rotation(float)
	{
		//
		//
		//
		m_constant_matrix_buffer->Map(0, nullptr, (void**)&m_matrix);
	}
}