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

	//ルートシグネチャの生成
	ID3D12RootSignature* create_root_signature(ID3D12Device* device)
	{
		ID3D12RootSignature* rootSignature = nullptr;

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;

		auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
		if (FAILED(result)) {
			std::cout << __func__ << " is failed : D3D12SerializeRootSignature ";

			//エラー内容
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

	//グラフィックスパイプラインの生成
	ID3D12PipelineState* create_graphics_pipline_state(ID3D12Device* device,ID3DBlob* vertexShader, ID3DBlob* pixelShader, ID3D12RootSignature* rootSignature)
	{

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};
		graphicsPipelineDesc.pRootSignature = nullptr;
		graphicsPipelineDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
		graphicsPipelineDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
		graphicsPipelineDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
		graphicsPipelineDesc.PS.BytecodeLength = pixelShader->GetBufferSize();

		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff

		//
		graphicsPipelineDesc.BlendState.AlphaToCoverageEnable = false;
		graphicsPipelineDesc.BlendState.IndependentBlendEnable = false;

		D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc{};

		//ひとまず加算や乗算やαブレンディングは使用しない
		renderTargetBlendDesc.BlendEnable = false;
		renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		//ひとまず論理演算は使用しない
		renderTargetBlendDesc.LogicOpEnable = false;

		graphicsPipelineDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;


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

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		};
		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputLayout;//レイアウト先頭アドレス
		graphicsPipelineDesc.InputLayout.NumElements = _countof(inputLayout);//レイアウト配列数

		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成

		graphicsPipelineDesc.NumRenderTargets = 1;//今は１つのみ
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0～1に正規化されたRGBA

		graphicsPipelineDesc.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
		graphicsPipelineDesc.SampleDesc.Quality = 0;//クオリティは最低

		//ルートシグネチャ
		graphicsPipelineDesc.pRootSignature = rootSignature;

		ID3D12PipelineState* graphicsPipline = nullptr;
		if (FAILED(device->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&graphicsPipline))))
		{
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}

		return graphicsPipline;
	}

	//ビューポートの取得
	D3D12_VIEWPORT get_viewport(unsigned int width, unsigned int height)
	{
		D3D12_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(width);//出力先の幅(ピクセル数)
		viewport.Height = static_cast<float>(height);//出力先の高さ(ピクセル数)
		viewport.TopLeftX = 0;//出力先の左上座標X
		viewport.TopLeftY = 0;//出力先の左上座標Y
		viewport.MaxDepth = 1.0f;//深度最大値
		viewport.MinDepth = 0.0f;//深度最小値

		return viewport;
	}

	//シザー矩形の取得
	D3D12_RECT get_scissor_rect(unsigned int width, unsigned int height)
	{
		D3D12_RECT scissorrect{};
		scissorrect.top = 0;//切り抜き上座標
		scissorrect.left = 0;//切り抜き左座標
		scissorrect.right = scissorrect.left + width;//切り抜き右座標
		scissorrect.bottom = scissorrect.top + height;//切り抜き下座標

		return scissorrect;
	}

}