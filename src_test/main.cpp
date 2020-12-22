#include"window.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/double_buffer.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/pipeline_state.hpp"
#include"DirectX12/shader.hpp"
#include<memory>

#include<iostream>

int main()
{
	constexpr float window_width = 400.f;
	constexpr float window_height = 400.f;
	auto hwnd = ichi::create_window(L"aaaaa", window_width, window_height);

	ichi::device device{};
	if (!device.initialize()) {
		std::cout << "device is failed\n";
		return 0;
	}

	auto commList = device.create_command_list();
	if (!commList) {
		std::cout << "comList is failed\n";
		return 0;
	}

	auto doubleBuffer = device.create_double_buffer(hwnd, commList);
	if (!doubleBuffer) {
		std::cout << "douebl is failed\n";
		delete commList;
		return 0;
	}

	//シェーダ
	auto vertShaderBlob = ichi::create_shader_blob(L"shader/VertexShader1.hlsl", "main", "vs_5_0");
	auto pixcShaderBlob = ichi::create_shader_blob(L"shader/PixelShader1.hlsl", "main", "ps_5_0");

	
	auto pipelineState = device.create_pipline_state(vertShaderBlob,pixcShaderBlob);
	if (!pipelineState) {
		std::cout << "pipe is failed\n";
		delete commList;
		delete doubleBuffer;
		return 0;
	}
	

	D3D12_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(window_width);//出力先の幅(ピクセル数)
	viewport.Height = static_cast<float>(window_height);//出力先の高さ(ピクセル数)
	viewport.TopLeftX = 0;//出力先の左上座標X
	viewport.TopLeftY = 0;//出力先の左上座標Y
	viewport.MaxDepth = 1.0f;//深度最大値
	viewport.MinDepth = 0.0f;//深度最小値
	
	D3D12_RECT scissorrect{};
	scissorrect.top = 0;//切り抜き上座標
	scissorrect.left = 0;//切り抜き左座標
	scissorrect.right = scissorrect.left + window_width;//切り抜き右座標
	scissorrect.bottom = scissorrect.top + window_height;//切り抜き下座標

	commList->get()->SetPipelineState(pipelineState->get());

	while (ichi::update_window()) {

		//順番注意
		//今のところ
		doubleBuffer->begin_resource_barrior(commList);
		auto rtvH = doubleBuffer->get_render_target(&device);
		
		commList->get()->OMSetRenderTargets(1, &rtvH, false, nullptr);

		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//黄色
		commList->get()->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);

		doubleBuffer->end_resource_barrior(commList);

		commList->get()->Close();

		commList->execute();

		commList->get_allocator()->Reset();
		commList->get()->Reset(commList->get_allocator(), pipelineState->get());
		
		doubleBuffer->flip();
		
	}

	if (commList)
		delete commList;
	
	if (doubleBuffer)
		delete doubleBuffer;

	vertShaderBlob->Release();
	pixcShaderBlob->Release();

	if (pipelineState)
		delete pipelineState;
		
	

	return 0;
}