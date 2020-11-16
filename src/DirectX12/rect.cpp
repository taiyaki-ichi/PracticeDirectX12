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

		//リソースの確保
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
			{-0.5f,-0.5f,0.0f},//左下
			{-0.5f,0.5f,0.0f} ,//左上
			{0.5f,-0.5f,0.0f} ,//右下
			{0.5f,0.5f,0.0f} ,//右上
		};


		//頂点
		resdesc.Width = sizeof(v);
		device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertex_buffer));

		m_vertex_buffer->Map(0, nullptr, (void**)&v);


		//インデックス
		unsigned int i[] = { 0,1,2, 2,1,3 };
		//リソース
		resdesc.Width = sizeof(i);
		device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_index_buffer));

		m_index_buffer->Map(0, nullptr, (void**)&i);

		//行列
		//アライメント忘れるな
		resdesc.Width = (sizeof(m_matrix) + 0xff) & ~0xff;
		device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constant_matrix_buffer));


		m_constant_matrix_buffer->Map(0, nullptr, (void**)&m_matrix);

		//色
		float c[] = { 0,1,1,1 };
		//アライメント忘れるな
		resdesc.Width = (sizeof(c) + 0xff) & ~0xff;
		device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constant_color_buffer));

		m_constant_matrix_buffer->Map(0, nullptr, (void**)&c);
		

		//ビュー
		m_vertex_view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
		m_vertex_view.SizeInBytes = sizeof(v);
		m_vertex_view.StrideInBytes = sizeof(v[0]);
		//ビュー
		m_index_view.BufferLocation = m_index_buffer->GetGPUVirtualAddress();
		m_index_view.SizeInBytes = sizeof(i);
		m_index_view.Format = DXGI_FORMAT_R16_UINT;
		
		//シェーダオブジェクトの生成
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
		//頂点
		HRESULT result;
		result = D3DCompileFromFile(
			L"shader/RectVertexShader.hlsl",
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "vs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, &m_vert_shader_blob, &errorBlob);
		//失敗した場合
		if (FAILED(result))
			errorFunc(result, __func__, errorBlob);

		//ピクセル
		result = D3DCompileFromFile(
			L"shader/RectPixcelShader.hlsl",
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", "ps_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0, &m_pixcel_shader_blob, &errorBlob);
		//失敗した場合
		if (FAILED(result))
			errorFunc(result, __func__, errorBlob);

		
		//行列と色
		D3D12_DESCRIPTOR_RANGE discriptorRangeDesc[2]{};
		//行列
		discriptorRangeDesc[0].NumDescriptors = 1;
		discriptorRangeDesc[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		discriptorRangeDesc[0].BaseShaderRegister = 0;
		discriptorRangeDesc[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		//色
		discriptorRangeDesc[1].NumDescriptors = 1;
		discriptorRangeDesc[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		discriptorRangeDesc[1].BaseShaderRegister = 1;
		discriptorRangeDesc[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		//ルートパラム(ディスクリプタテーブル)
		D3D12_ROOT_PARAMETER rootParam[2]{};
		//行列
		rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[0].DescriptorTable.pDescriptorRanges = &discriptorRangeDesc[0];
		rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		//色
		rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[1].DescriptorTable.pDescriptorRanges = &discriptorRangeDesc[1];
		rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


		//ルートシグネチャ
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		rootSignatureDesc.pParameters = &rootParam[0];
		rootSignatureDesc.NumParameters = 2;
		rootSignatureDesc.NumStaticSamplers = 0;

		ID3DBlob* root_signature_blob = nullptr;
		result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,&root_signature_blob, &errorBlob);
		if (FAILED(result)) {
			std::cout << __func__ << " is failed : D3D12SerializeRootSignature ";
			//エラー内容
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


		//ラスタライザー
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

		//ブレンドステート設定用構造体
		D3D12_BLEND_DESC blend{};
		blend.AlphaToCoverageEnable = TRUE;
		blend.IndependentBlendEnable = FALSE;
		blend.RenderTarget[0] = renderBlend;

		//頂点情報のレイアウト
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 }
		};

		//パイプライン
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};
		graphicsPipelineDesc.VS.pShaderBytecode = m_vert_shader_blob->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = m_vert_shader_blob->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = m_pixcel_shader_blob->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = m_pixcel_shader_blob->GetBufferSize();
		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff
		graphicsPipelineDesc.BlendState.AlphaToCoverageEnable = false;
		graphicsPipelineDesc.BlendState.IndependentBlendEnable = false;
		graphicsPipelineDesc.BlendState = blend;
		graphicsPipelineDesc.RasterizerState.MultisampleEnable = false;//まだアンチェリは使わない
		graphicsPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリングしない
		graphicsPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//中身を塗りつぶす
		graphicsPipelineDesc.RasterizerState.DepthClipEnable = true;//深度方向のクリッピングは有効に
		graphicsPipelineDesc.RasterizerState.FrontCounterClockwise = false;
		graphicsPipelineDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		graphicsPipelineDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.AntialiasedLineEnable = false;
		graphicsPipelineDesc.RasterizerState.ForcedSampleCount = 0;
		graphicsPipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		graphicsPipelineDesc.DepthStencilState.DepthEnable = false;
		graphicsPipelineDesc.DepthStencilState.StencilEnable = false;
		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputLayout;//レイアウト先頭アドレス
		graphicsPipelineDesc.InputLayout.NumElements = _countof(inputLayout);//レイアウト配列数
		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成
		graphicsPipelineDesc.NumRenderTargets = 1;//今は１つのみ
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0～1に正規化されたRGBA
		graphicsPipelineDesc.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
		graphicsPipelineDesc.SampleDesc.Quality = 0;//クオリティは最低
		graphicsPipelineDesc.pRootSignature = m_root_signature;

		

		result = device->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_graphics_pipeline_state));
		if (FAILED(result))
			std::cout << __func__ << " is failed : CreateGraphicsPipelineState : "<<result<<" \n";


		//ディスクリプタヒープ
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descHeapDesc.NodeMask = 0;
		//マトリックス、色の三つ
		descHeapDesc.NumDescriptors = 2;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//生成
		device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_descriptor_heap));

		//デスクリプタの先頭ハンドル
		auto basicHeapHandle = m_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
		
		//定数
		D3D12_CONSTANT_BUFFER_VIEW_DESC constantMatrixDesc{};
		constantMatrixDesc.BufferLocation = m_constant_matrix_buffer->GetGPUVirtualAddress();
		constantMatrixDesc.SizeInBytes = static_cast<UINT>(m_constant_matrix_buffer->GetDesc().Width);
		//定数バッファビューの作成
		device->CreateConstantBufferView(&constantMatrixDesc, basicHeapHandle);

		basicHeapHandle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//色
		D3D12_CONSTANT_BUFFER_VIEW_DESC constantColorDesc{};
		constantColorDesc.BufferLocation = m_constant_color_buffer->GetGPUVirtualAddress();
		constantColorDesc.SizeInBytes = static_cast<UINT>(m_constant_color_buffer->GetDesc().Width);
		//定数バッファビューの作成
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