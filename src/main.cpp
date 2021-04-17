#include"window.hpp"
#include"window_size.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/double_buffer.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/shader.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"include/load_pmx.hpp"
#include"DirectX12/depth_buffer.hpp"
#include<DirectXMath.h>
#include<memory>
#include<array>

#include"mmd_model.hpp"
#include"mmd_model_renderer.hpp"
#include"scene_data.hpp"

#include"perapolygon.hpp"
#include"perapolygon_renderer.hpp"


#include<iostream>


//�V���h�E�}�b�v�p�̐[�x�o�b�t�@�������Ɏg�p
constexpr uint32_t shadow_difinition = 1024;

int main()
{
	//using���Ȃ��Ɖ��Z�q���g���Ȃ�
	using namespace DirectX;

	auto hwnd = DX12::create_window(L"directx", window_width, window_height);

	//�Ƃ肠�����X�}�[�g�|�C���^�g���Ă���
	DX12::device device{};
	if (!device.initialize()) {
		std::cout << "device is failed\n";
		return 0;
	}

	DX12::command_list commList{};
	if (!commList.initialize(&device)) {
		std::cout << "comList is failed\n";
		return 0;
	}

	DX12::double_buffer doubleBuffer{};
	if (!doubleBuffer.initialize(&device, hwnd, &commList)) {
		std::cout << "douebl is failed\n";
		return 0;
	}

	//
	//���ʂ̐[�x�ƃ��C�g�̐[�x�p
	//
	DX12::depth_buffer<2> depthBuffer{};
	if (!depthBuffer.initialize(&device, std::make_pair(window_width, window_height), std::make_pair(1024u, 1024u))) {
		std::cout << "depth is failed\n";
		return false;
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
	DirectX::XMFLOAT3 eye{ 0,7,-6 };
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

	//���s���C�g�̌���
	//�E��������
	DirectX::XMFLOAT3 parallelLightVec{ 1.f,-1.f,1.f };
	//�ꊇ�̕��@���������
	parallelLightVec.x *= -1.f;
	parallelLightVec.y *= -1.f;
	parallelLightVec.z *= -1.f;
	//�e�������镽�ʂ̕������̃f�[�^
	DirectX::XMFLOAT4 planeVec{ 0.f,1.f,0.f,0.f };
	//���ۂ̉e�s��
	DirectX::XMMATRIX shadow = DirectX::XMMatrixShadow(
		DirectX::XMLoadFloat4(&planeVec), DirectX::XMLoadFloat3(&parallelLightVec));

	//���C�g�̃|�W�V����
	auto lightPos = XMLoadFloat3(&target) + XMVector3Normalize(XMLoadFloat3(&parallelLightVec))
		* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];

	auto lightCamera = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up))* XMMatrixOrthographicLH(50, 50, -10.f, 100.f);


	//
	//mmd���f���̒��_���
	//���_���C�A�E�g�ƃV�F�[�_��ύX����̂Œ���
	//


	DX12::mmd_model mmdModel{};
	{
		auto result = MMDL::load_pmx("../../mmd/Paimon/�h��.pmx");
		if (result)
		{
			auto& model = std::get<MMDL::pmx_model<std::wstring>>(result.value());

			if (!mmdModel.initialize(&device, model, &commList, depthBuffer.get_resource(1))) {
				std::cout << "mmd model3 init is failed\n";
				return 0;
			}
		}
		else {
			std::cout << "mmd model3 load is failed\n";
			return 0;
		}
	}

	DX12::mmd_model_renderer mmdModelRenderer{};
	if (!mmdModelRenderer.initialize(&device)) {
		std::cout << "mmd model renderer init is failed\n";
		return 0;
	}



	//
	//�؂�|���S��
	//
	DX12::perapolygon perapolygon{};
	if (!perapolygon.initialize(&device,depthBuffer.get_resource(0))) {
		std::cout << "pera false";
		return 0;
	}
	DX12::perapolygon_renderer perapolygonRenderer{};
	if (!perapolygonRenderer.initialize(&device)) {
		std::cout << "pera renderer init is failed\n";
		return 0;
	}



	//
	//���C�g�[�x�p
	//
	D3D12_VIEWPORT lightDepthViewport{};
	lightDepthViewport.Width = static_cast<float>(shadow_difinition);//�o�͐�̕�(�s�N�Z����)
	lightDepthViewport.Height = static_cast<float>(shadow_difinition);//�o�͐�̍���(�s�N�Z����)
	lightDepthViewport.TopLeftX = 0;//�o�͐�̍�����WX
	lightDepthViewport.TopLeftY = 0;//�o�͐�̍�����WY
	lightDepthViewport.MaxDepth = 1.0f;//�[�x�ő�l
	lightDepthViewport.MinDepth = 0.0f;//�[�x�ŏ��l
	
	D3D12_RECT lightDepthScissorRect{};
	lightDepthScissorRect.top = 0;//�؂蔲������W
	lightDepthScissorRect.left = 0;//�؂蔲�������W
	lightDepthScissorRect.right = lightDepthScissorRect.left + shadow_difinition;//�؂蔲���E���W
	lightDepthScissorRect.bottom = lightDepthScissorRect.top + shadow_difinition;//�؂蔲�������W

	mmdModel.rotation_bone(16, DirectX::XMMatrixRotationZ(-DirectX::XM_PIDIV2));

	while (DX12::update_window()) {
		
		mmdModel.map_scene_data({ view,proj,lightCamera, shadow, eye });
		mmdModel.update();

		XMVECTOR det;
		auto invProj = XMMatrixInverse(&det, proj);
		perapolygon.map_scene_data({ view,proj,invProj,lightCamera,shadow,eye });

		//
		//���̃f�B�v�X�`��
		//

		depthBuffer.clear(&commList, 1);

		commList.set_viewport(lightDepthViewport);
		commList.set_scissor_rect(lightDepthScissorRect);

		commList.set_render_target(0, nullptr, depthBuffer.get_cpu_handle(1));

		mmdModelRenderer.preparation_for_drawing_light_depth(&commList);
		mmdModel.draw_light_depth(&commList);

		//
		//mmd���؂�|���S���֕`��
		//
		
		commList.set_viewport(viewport);
		commList.set_scissor_rect(scissorrect);

		perapolygon.all_resource_barrior(&commList, D3D12_RESOURCE_STATE_RENDER_TARGET);
		perapolygon.clear(&commList);

		auto [renderNum, perapolygonHandle] = perapolygon.get_render_target_info();
		commList.set_render_target(renderNum, perapolygonHandle, depthBuffer.get_cpu_handle(0));

		depthBuffer.clear(&commList, 0);

		mmdModelRenderer.preparation_for_drawing(&commList);
		mmdModel.draw(&commList);

		perapolygonRenderer.preparation_for_drawing_for_blur(&commList);
		perapolygon.draw_shrink_texture_for_blur(&commList);

		perapolygon.all_resource_barrior(&commList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//
		//SSAO�p�̃f�[�^��`��
		//
		commList.set_viewport(viewport);
		commList.set_scissor_rect(scissorrect);

		perapolygon.ssao_resource_barrior(&commList, D3D12_RESOURCE_STATE_RENDER_TARGET);
		
		commList.set_render_target(perapolygon.get_ssao_cpu_handle());

		perapolygonRenderer.preparation_for_drawing_for_SSAO(&commList);

		perapolygon.draw(&commList);

		perapolygon.ssao_resource_barrior(&commList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//
		//�؂�|���S�����o�b�N�o�b�t�@�ɕ`��
		//

		commList.set_viewport(viewport);
		commList.set_scissor_rect(scissorrect);

		//�؂�|���S���̕`�ʂȂ̂Ő[�x�o�b�t�@�͂���Ȃ�
		doubleBuffer.barrior_to_backbuffer(&commList, D3D12_RESOURCE_STATE_RENDER_TARGET);

		commList.set_render_target(doubleBuffer.get_backbuffer_cpu_handle());

		doubleBuffer.clear_back_buffer(&commList);

		perapolygonRenderer.preparation_for_drawing(&commList);
		perapolygon.draw(&commList);

		doubleBuffer.barrior_to_backbuffer(&commList, D3D12_RESOURCE_STATE_PRESENT);

		commList.close();
		commList.execute();

		commList.clear();

		doubleBuffer.flip();
	}
	
	return 0;
}