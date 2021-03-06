#include"perapolygon_renderer.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/shader.hpp"

#include<iostream>

namespace DX12
{
	perapolygon_renderer::~perapolygon_renderer()
	{
		if (m_blur_pipeline_state)
			m_blur_pipeline_state->Release();
		if (m_pipeline_state)
			m_pipeline_state->Release();
		if (m_root_signature)
			m_root_signature->Release();
		if (m_SSAO_pipeline_state)
			m_SSAO_pipeline_state->Release();
	}

	bool perapolygon_renderer::initialize(device* device)
	{
		////////////////////////////////////////////////////////////////////////////////////////
		//ルートシグネチャの作製
		////////////////////////////////////////////////////////////////////////////////////////

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//テクスチャ
		D3D12_DESCRIPTOR_RANGE range[3]{};
		range[0].NumDescriptors = 6;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[0].BaseShaderRegister = 0;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//定数
		range[1].NumDescriptors = 1;
		range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range[1].BaseShaderRegister = 0;
		range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//ディプス
		range[2].NumDescriptors = 1;
		range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[2].BaseShaderRegister = 6;
		range[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		D3D12_ROOT_PARAMETER rootparam{};
		rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootparam.DescriptorTable.pDescriptorRanges = range;
		rootparam.DescriptorTable.NumDescriptorRanges = 3;

		rootSignatureDesc.NumParameters = 1;
		rootSignatureDesc.pParameters = &rootparam;

		D3D12_STATIC_SAMPLER_DESC samplerDesc{};
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
		samplerDesc.MinLOD = 0.0f;//ミップマップ最小値
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

		rootSignatureDesc.NumStaticSamplers = 1;
		rootSignatureDesc.pStaticSamplers = &samplerDesc;

		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		if (FAILED(result)) {
			std::cout << "perapolygon D3D12SerializeRootSignature is failed : D3D12SerializeRootSignature ";

			//エラー内容
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			std::cout << " : " << errstr << "\n";
		}

		result = device->get()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature));
		if (FAILED(result)) {
			std::cout << "perapolygon CreateRootSignature is falied : CreateRootSignature\n";
		}

		rootSigBlob->Release();



		////////////////////////////////////////////////////////////////////////////////////////
		//パイプラインステートたちの作製
		////////////////////////////////////////////////////////////////////////////////////////
		
		auto vertexShader = create_shader_blob(L"shader/peraVertexShader.hlsl", "main", "vs_5_0");
		auto pixelShader = create_shader_blob(L"shader/peraPixelShader.hlsl", "main", "ps_5_0");
		auto blurPixelShader = create_shader_blob(L"shader/peraPixelShader.hlsl", "BlurPS", "ps_5_0");
		auto ssaoPixelShader = create_shader_blob(L"shader/peraPixelShader.hlsl", "SsaoPS", "ps_5_0");


		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		graphicsPipelineDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = pixelShader->GetBufferSize();

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff

		graphicsPipelineDesc.BlendState.AlphaToCoverageEnable = false;
		graphicsPipelineDesc.BlendState.IndependentBlendEnable = false;


		//ラスタライザーステート設定用構造体の設定
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
		//とりあえず0だけにしておく
		blend.RenderTarget[0] = renderBlend;

		graphicsPipelineDesc.BlendState = blend;

		graphicsPipelineDesc.RasterizerState.MultisampleEnable = false;//まだアンチェリは使わない
		graphicsPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリングしない
		graphicsPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//中身を塗りつぶす
		graphicsPipelineDesc.RasterizerState.DepthClipEnable = true;//深度方向のクリッピングは有効に

		//残り
		graphicsPipelineDesc.RasterizerState.FrontCounterClockwise = false;
		graphicsPipelineDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		graphicsPipelineDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		graphicsPipelineDesc.RasterizerState.AntialiasedLineEnable = false;
		graphicsPipelineDesc.RasterizerState.ForcedSampleCount = 0;
		graphicsPipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		graphicsPipelineDesc.DepthStencilState.DepthEnable = false;
		graphicsPipelineDesc.DepthStencilState.StencilEnable = false;

		//
		//頂点情報のレイアウト
		//ぺらポリゴン用にカスタム
		//
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		};


		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputLayout;//レイアウト先頭アドレス
		graphicsPipelineDesc.InputLayout.NumElements = _countof(inputLayout);//レイアウト配列数

		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成

		graphicsPipelineDesc.NumRenderTargets = 1;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0〜1に正規化されたRGBA

		graphicsPipelineDesc.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
		graphicsPipelineDesc.SampleDesc.Quality = 0;//クオリティは最低

		//ルートシグネチャ
		graphicsPipelineDesc.pRootSignature = m_root_signature;

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_pipeline_state)))){
			std::cout << "pera CreateGraphicsPipelineState is failed\n";
		}

		//ピクセルシェーダの設定
		graphicsPipelineDesc.PS.pShaderBytecode = blurPixelShader->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = blurPixelShader->GetBufferSize();

		graphicsPipelineDesc.DepthStencilState.DepthEnable = false;

		graphicsPipelineDesc.NumRenderTargets = 2;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		graphicsPipelineDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;


		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_blur_pipeline_state)))){
			std::cout << "pera CreateGraphicsPipelineState 2 is failed\n";
		}

		graphicsPipelineDesc.NumRenderTargets = 1;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
		graphicsPipelineDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
		graphicsPipelineDesc.BlendState.RenderTarget[0].BlendEnable = false;

		graphicsPipelineDesc.PS.pShaderBytecode = ssaoPixelShader->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = ssaoPixelShader->GetBufferSize();

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_SSAO_pipeline_state)))) {
			std::cout << "pera CreateGraphicsPipelineState 3 is failed\n";
		}

		vertexShader->Release();
		pixelShader->Release();
		blurPixelShader->Release();
		ssaoPixelShader->Release();

		if (m_blur_pipeline_state && m_pipeline_state && m_root_signature)
			return true;
		else
			return false;

	}

	void perapolygon_renderer::preparation_for_drawing(command_list* cl)
	{
		cl->get()->SetPipelineState(m_pipeline_state);
		cl->get()->SetGraphicsRootSignature(m_root_signature);

		cl->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}

	void perapolygon_renderer::preparation_for_drawing_for_blur(command_list* cl)
	{
		cl->get()->SetPipelineState(m_blur_pipeline_state);
		cl->get()->SetGraphicsRootSignature(m_root_signature);

		cl->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}

	void perapolygon_renderer::preparation_for_drawing_for_SSAO(command_list* cl)
	{
		cl->get()->SetPipelineState(m_SSAO_pipeline_state);
		cl->get()->SetGraphicsRootSignature(m_root_signature);

		cl->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}

	perapolygon_renderer::perapolygon_renderer(perapolygon_renderer&& r) noexcept
	{
		m_blur_pipeline_state = r.m_blur_pipeline_state;
		m_pipeline_state = r.m_pipeline_state;
		m_root_signature = r.m_root_signature;
		r.m_blur_pipeline_state = nullptr;
		r.m_pipeline_state = nullptr;
		r.m_root_signature = nullptr;
	}

	perapolygon_renderer& perapolygon_renderer::operator=(perapolygon_renderer&& r) noexcept
	{
		m_blur_pipeline_state = r.m_blur_pipeline_state;
		m_pipeline_state = r.m_pipeline_state;
		m_root_signature = r.m_root_signature;
		r.m_blur_pipeline_state = nullptr;
		r.m_pipeline_state = nullptr;
		r.m_root_signature = nullptr;
		return *this;
	}



}