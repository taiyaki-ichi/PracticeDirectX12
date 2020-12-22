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

	//�V�F�[�_
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
	viewport.Width = static_cast<float>(window_width);//�o�͐�̕�(�s�N�Z����)
	viewport.Height = static_cast<float>(window_height);//�o�͐�̍���(�s�N�Z����)
	viewport.TopLeftX = 0;//�o�͐�̍�����WX
	viewport.TopLeftY = 0;//�o�͐�̍�����WY
	viewport.MaxDepth = 1.0f;//�[�x�ő�l
	viewport.MinDepth = 0.0f;//�[�x�ŏ��l
	
	D3D12_RECT scissorrect{};
	scissorrect.top = 0;//�؂蔲������W
	scissorrect.left = 0;//�؂蔲�������W
	scissorrect.right = scissorrect.left + window_width;//�؂蔲���E���W
	scissorrect.bottom = scissorrect.top + window_height;//�؂蔲�������W

	commList->get()->SetPipelineState(pipelineState->get());

	while (ichi::update_window()) {

		//���Ԓ���
		//���̂Ƃ���
		doubleBuffer->begin_resource_barrior(commList);
		auto rtvH = doubleBuffer->get_render_target(&device);
		
		commList->get()->OMSetRenderTargets(1, &rtvH, false, nullptr);

		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//���F
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