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
		//ルートシグネチャの作製
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		D3D12_DESCRIPTOR_RANGE range[]{ {},{} ,{},{} };
		//頂点情報
		range[0].NumDescriptors = 1;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range[0].BaseShaderRegister = 0;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//マテリアるの基本データ
		range[1].NumDescriptors = 1;
		range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range[1].BaseShaderRegister = 1;
		range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//マテリアルのスフィアとかトゥーン
		range[2].NumDescriptors = 4;
		range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[2].BaseShaderRegister = 0;
		range[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//ライトの深度用
		range[3].NumDescriptors = 1;
		range[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[3].BaseShaderRegister = 4;
		range[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		D3D12_ROOT_PARAMETER rootparam[]{ {},{},{} };
		rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[0].DescriptorTable.pDescriptorRanges = &range[0];//デスクリプタレンジのアドレス
		rootparam[0].DescriptorTable.NumDescriptorRanges = 1;//デスクリプタレンジ数
		rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//全てのシェーダから見える

		//マテリアルごとにGPUハンドルを一緒にずらすためまとめる
		rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[1].DescriptorTable.pDescriptorRanges = &range[1];//デスクリプタレンジのアドレス
		rootparam[1].DescriptorTable.NumDescriptorRanges = 2;//デスクリプタレンジ数
		rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//全てのシェーダから見える

		rootparam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootparam[2].DescriptorTable.pDescriptorRanges = &range[3];//デスクリプタレンジのアドレス
		rootparam[2].DescriptorTable.NumDescriptorRanges = 1;//デスクリプタレンジ数
		rootparam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//全てのシェーダから見える

		//まとめる
		rootSignatureDesc.pParameters = rootparam;//ルートパラメータの先頭アドレス
		rootSignatureDesc.NumParameters = 3;//ルートパラメータ数

		//通常のと、トゥーン用
		D3D12_STATIC_SAMPLER_DESC samplerDesc[]{ {},{} ,{} };
		samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
		samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
		samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
		samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
		samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
		samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
		samplerDesc[0].MinLOD = 0.0f;//ミップマップ最小値
		samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
		samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

		samplerDesc[1] = samplerDesc[0];//変更点以外をコピー
		samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//
		samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc[1].ShaderRegister = 1;

		samplerDesc[2] = samplerDesc[1];
		samplerDesc[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplerDesc[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;//比較結果をリニア補完
		samplerDesc[2].MaxAnisotropy = 1;//深度傾斜を有効に
		samplerDesc[2].ShaderRegister = 2;

		//まとめる
		rootSignatureDesc.pStaticSamplers = samplerDesc;
		rootSignatureDesc.NumStaticSamplers = 3;


		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		if (FAILED(result)) {
			std::cout << "mmd D3D12SerializeRootSignature is failed ";

			//エラー内容
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
		//グラフィックスパイプラインの作製
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		auto vertShaderBlob = create_shader_blob(L"shader/VertexShader.hlsl", "main", "vs_5_0");
		auto pixcShaderBlob = create_shader_blob(L"shader/PixelShader.hlsl", "main", "ps_5_0");
		auto shadowVertShaderBlob = create_shader_blob(L"shader/VertexShader.hlsl", "shadowVS", "vs_5_0");


		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		graphicsPipelineDesc.VS.pShaderBytecode = vertShaderBlob->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = vertShaderBlob->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = pixcShaderBlob->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = pixcShaderBlob->GetBufferSize();

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff

		graphicsPipelineDesc.BlendState.AlphaToCoverageEnable = false;
		graphicsPipelineDesc.BlendState.IndependentBlendEnable = false;


		//ラスタライザーステート設定用構造体の設定
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

		//頂点情報のレイアウト
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },

			{ "BONE_TYPE_FLAG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			//unsigned short[4]
			{ "BONENO",0,DXGI_FORMAT_R16G16B16A16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			//float[4]...32じゃあなくて16でもよさそうだけど
			{ "WEIGHT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			//SDEF用
			//配列にするやり方とかアンのかね
			{ "SDEF_ONE",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{ "SDEF_TWO",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{ "SDEF_TREE",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		};


		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputLayout;//レイアウト先頭アドレス
		graphicsPipelineDesc.InputLayout.NumElements = _countof(inputLayout);//レイアウト配列数

		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成

		graphicsPipelineDesc.NumRenderTargets = 3;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0～1に正規化されたRGBA
		graphicsPipelineDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
		graphicsPipelineDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;

		graphicsPipelineDesc.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
		graphicsPipelineDesc.SampleDesc.Quality = 0;//クオリティは最低

		//ルートシグネチャ
		graphicsPipelineDesc.pRootSignature = m_root_signature;

		if (FAILED(device->get()->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&m_pipeline_state))))
		{
			std::cout << "mmd CreateGraphicsPipelineState is failed\n";
		}


		//ライト深度用のパイプラインステート
		ID3D12PipelineState* shadowPipelineState = nullptr;

		graphicsPipelineDesc.VS.pShaderBytecode = shadowVertShaderBlob->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = shadowVertShaderBlob->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = nullptr;
		graphicsPipelineDesc.PS.BytecodeLength = 0;

		//レンダーターゲット必要なし
		graphicsPipelineDesc.NumRenderTargets = 0;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		//[1]のクリアを忘れずに
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