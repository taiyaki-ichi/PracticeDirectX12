#include"window.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/double_buffer.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/pipeline_state.hpp"
#include"DirectX12/shader.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/index_buffer.hpp"
#include"DirectX12/constant_buffer_resource.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/texture_shader_resource.hpp"
#include"include/load_pmx.hpp"
#include"DirectX12/depth_buffer.hpp"
#include<DirectXMath.h>
#include<memory>
#include<array>

#include"mmd_model.hpp"
#include"scene_data.hpp"


#include<iostream>

int main()
{
	constexpr unsigned int window_width = 800;
	constexpr unsigned int window_height = 600;
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
	auto vertShaderBlob = ichi::create_shader_blob(L"shader/VertexShader.hlsl", "main", "vs_5_0");
	auto pixcShaderBlob = ichi::create_shader_blob(L"shader/PixelShader.hlsl", "main", "ps_5_0");

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

	//
	//viewproj
	//
	DirectX::XMFLOAT3 eye{ 0,5,-5 };
	DirectX::XMFLOAT3 target{ 0,5,0 };
	DirectX::XMFLOAT3 up{ 0,1,0 };
	auto view = DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&eye), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
	auto proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV2,
		static_cast<float>(window_width) / static_cast<float>(window_height),
		1.f,
		100.f
	);
	//auto viewproj = view * proj;

	//
	//���f���{�̂���]��������ړ��������肷��s��
	//
	DirectX::XMMATRIX worldMat = DirectX::XMMatrixIdentity();


	//
	//mmd���f���̒��_���
	//���_���C�A�E�g�ƃV�F�[�_��ύX����̂Œ���
	//
	//���f���̓ǂݍ���
	//
	auto mmdModel = std::shared_ptr<ichi::mmd_model>{};

	auto modelIf = MMDL::load_pmx("../../mmd/Paimon/�h��.pmx");
	if (modelIf)
	{
		//�Ƃ肠����
		auto&& model = std::get<MMDL::pmx_model<std::wstring>>(modelIf.value());

		mmdModel = std::shared_ptr<ichi::mmd_model>{
			device->create<ichi::mmd_model>(model,commList.get())
		};

	}
	else {
		std::cout << "model loaf failed\n";
		return 0;
	}


	//
	//�ł��Ղ�
	//
	auto depthBuffer = std::shared_ptr<ichi::depth_buffer>{
		device->create<ichi::depth_buffer>(window_width,window_height)
	};


	
	while (ichi::update_window()) {
		
		//��]�̌v�Z
		worldMat *= DirectX::XMMatrixRotationRollPitchYaw(0.f, 0.01f, 0.f);

		//mmdModel->map_world_mat(worldMat);
		//mmdModel->map_viewproj_mat(viewproj);

		mmdModel->map_scene_data({ worldMat,view,proj,eye });

		doubleBuffer->begin_drawing_to_backbuffer(commList.get(), depthBuffer.get());

		doubleBuffer->clear_back_buffer(commList.get());
		depthBuffer->clear(commList.get());

		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);
		commList->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commList->get()->SetPipelineState(pipelineState->get());
		commList->get()->SetGraphicsRootSignature(pipelineState->get_root_signature());
		
		mmdModel->draw(commList.get());

		doubleBuffer->end_drawing_to_backbuffer(commList.get());

		commList->get()->Close();
		commList->execute();
		commList->clear(pipelineState.get());
		

		doubleBuffer->flip();

	}


	vertShaderBlob->Release();
	pixcShaderBlob->Release();

	return 0;
}