#include"window.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/double_buffer.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/pipeline_state.hpp"
#include"DirectX12/shader.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/index_buffer.hpp"
#include<DirectXMath.h>
#include<memory>


#include<iostream>

int main()
{
	constexpr float window_width = 400.f;
	constexpr float window_height = 400.f;
	auto hwnd = ichi::create_window(L"aaaaa", window_width, window_height);

	//����߂�ǂ��̂łƂ肠�����X�}�[�g�|�C���^�g���Ă���
	auto device = std::make_shared<ichi::device>();
	if (!device->initialize()) {
		std::cout << "device is failed\n";
		return 0;
	}

	auto commList = std::shared_ptr<ichi::command_list>(device->create<ichi::command_list>());
	if (!commList) {
		std::cout << "comList is failed\n";
		return 0;
	}

	auto doubleBuffer = std::shared_ptr<ichi::double_buffer>(device->create<ichi::double_buffer>(hwnd, commList.get()));
	if (!doubleBuffer) {
		std::cout << "douebl is failed\n";
		return 0;
	}

	//�V�F�[�_
	auto vertShaderBlob = ichi::create_shader_blob(L"shader/VertexShader1.hlsl", "main", "vs_5_0");
	auto pixcShaderBlob = ichi::create_shader_blob(L"shader/PixelShader1.hlsl", "main", "ps_5_0");

	auto pipelineState = std::shared_ptr<ichi::pipeline_state>(device->create<ichi::pipeline_state>(vertShaderBlob, pixcShaderBlob));
	if (!pipelineState) {
		std::cout << "pipe is failed\n";
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


	struct Vertex {
		DirectX::XMFLOAT3 pos;//XYZ���W
		DirectX::XMFLOAT2 uv;//UV���W
	};

	Vertex vertices[] = {
		{{-1.f,-1.f,0.0f},{0.0f,1.0f} },//����
		{{-1.f,1.f,0.0f} ,{0.0f,0.0f}},//����
		{{1.f,-1.f,0.0f} ,{1.0f,1.0f}},//�E��
		{{1.f,1.f,0.0f} ,{1.0f,0.0f}},//�E��
	};

	//auto vertBuffer = std::shared_ptr<ichi::vertex_buffer>{d}
	


	commList->get()->SetPipelineState(pipelineState->get());

	while (ichi::update_window()) {

		doubleBuffer->begin_drawing_to_backbuffer(commList.get());

		//�R�}���h���X�g�̃����o�ɂ܂Ƃ߂Ă��܂���
		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);




		doubleBuffer->end_drawing_to_backbuffer(commList.get());
		commList->get()->Close();

		commList->execute();

		commList->get_allocator()->Reset();
		commList->get()->Reset(commList->get_allocator(), pipelineState->get());
		
		doubleBuffer->flip();
		
	}

	vertShaderBlob->Release();
	pixcShaderBlob->Release();

	return 0;
}