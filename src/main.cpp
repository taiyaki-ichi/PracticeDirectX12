#include"window.hpp"
#include"window_size.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/double_buffer.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/shader.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/index_buffer.hpp"
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


#include<iostream>


//���ߗp
template<typename T,typename... Args>
std::shared_ptr<T> create_shared_ptr(DX12::device* device,Args&&... args) {
	return std::shared_ptr<T>{ device->create<T>(std::forward<Args>(args)...)};
}

//�V���h�E�}�b�v�p�̐[�x�o�b�t�@�������Ɏg�p
constexpr uint32_t shadow_difinition = 1024;

int main()
{

	auto hwnd = DX12::create_window(L"aaaaa", window_width, window_height);

	//�Ƃ肠�����X�}�[�g�|�C���^�g���Ă���
	auto device = std::make_shared<DX12::device>();
	if (!device->initialize()) {
		std::cout << "device is failed\n";
		return 0;
	}

	auto commList = create_shared_ptr<DX12::command_list>(device.get());
	if (!commList) {
		std::cout << "comList is failed\n";
		return 0;
	}

	auto doubleBuffer = create_shared_ptr<DX12::double_buffer>(device.get(), hwnd, commList.get());
	if (!doubleBuffer) {
		std::cout << "douebl is failed\n";
		return 0;
	}

	//
	//���ʂ̐[�x�ƃ��C�g�̐[�x�p
	//
	auto depthBuffer = std::make_unique<DX12::depth_buffer<2>>();
	if (!depthBuffer->initialize(device.get(), std::make_pair(window_width, window_height), std::make_pair(1024u, 1024u))) {
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

	//
	//���f���{�̂���]��������ړ��������肷��s��
	//
	DirectX::XMMATRIX worldMat = DirectX::XMMatrixIdentity();

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
	//using���Ȃ��Ɖ��Z�q���g���Ȃ�
	using namespace DirectX;
	auto lightPos = XMLoadFloat3(&target) + XMVector3Normalize(XMLoadFloat3(&parallelLightVec))
		* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];

	auto lightCamera = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up))* XMMatrixOrthographicLH(50, 50, -10.f, 100.f);


	//
	//mmd���f���̒��_���
	//���_���C�A�E�g�ƃV�F�[�_��ύX����̂Œ���
	//
	//3�̂̃��f���̓ǂݍ���
	//
	std::shared_ptr<DX12::mmd_model> mmdModel{};
	{
		auto result = MMDL::load_pmx("../../mmd/Paimon/�h��.pmx");
		if (result)
		{
			//�Ƃ肠����
			auto& model = std::get<MMDL::pmx_model<std::wstring>>(result.value());

			mmdModel = create_shared_ptr<DX12::mmd_model>(device.get(), model, commList.get(), depthBuffer->get_resource(1));
		}
		else {
			std::cout << "model load failed\n";
			return 0;
		}
	}

	std::shared_ptr<DX12::mmd_model> mmdModel2{};
	{
		auto result = MMDL::load_pmx("../../mmd/Qiqi/����.pmx");
		if (result)
		{
			//�Ƃ肠����
			auto& model = std::get<MMDL::pmx_model<std::wstring>>(result.value());

			mmdModel2 = create_shared_ptr<DX12::mmd_model>(device.get(), model, commList.get(), depthBuffer->get_resource(1));
		}
		else {
			std::cout << "model2 load failed\n";
			return 0;
		}
	}

	std::shared_ptr<DX12::mmd_model> mmdModel3{};
	{
		auto result = MMDL::load_pmx("../../mmd/Mona/���P1.0.pmx");
		if (result)
		{
			auto& model = std::get<MMDL::pmx_model<std::wstring>>(result.value());
			mmdModel3 = create_shared_ptr<DX12::mmd_model>(device.get(), model, commList.get(), depthBuffer->get_resource(1));
		}
		else {
			std::cout << "model3 load is failed\n";
			return 0;
		}
	}

	DX12::mmd_model_renderer mmdModelRenderer{};
	if (!mmdModelRenderer.initialize(device.get())) {
		std::cout << "mmd model renderer init is failed\n";
		return 0;
	}


	//
	//�؂�|���S��
	//
	auto perapolygon = std::make_unique<DX12::perapolygon>();
	if (!perapolygon->initialize(device.get(),depthBuffer->get_resource(0)->get())) {
		std::cout << "pera false";
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

	while (DX12::update_window()) {
		
		//��]�̌v�Z
		worldMat *= DirectX::XMMatrixRotationRollPitchYaw(0.f, 0.01f, 0.f);
	
		mmdModel->map_scene_data({ worldMat,view,proj,lightCamera, shadow, eye });
		mmdModel2->map_scene_data({ worldMat * DirectX::XMMatrixTranslation(5.f,0,5.f),view,proj,lightCamera, shadow, eye });
		mmdModel3->map_scene_data({ worldMat * DirectX::XMMatrixTranslation(-5.f,0,10.f),view,proj,lightCamera, shadow, eye });

		//
		//���̃f�B�v�X�`��
		//

		depthBuffer->clear(commList.get(), 1);

		commList->get()->RSSetViewports(1, &lightDepthViewport);
		commList->get()->RSSetScissorRects(1, &lightDepthScissorRect);

		auto lightDepthHandle = depthBuffer->get_cpu_handle(1);
		commList->get()->OMSetRenderTargets(0, nullptr, false, &lightDepthHandle);

		mmdModelRenderer.preparation_for_drawing_light_depth(commList.get());
		mmdModel->draw_light_depth(commList.get());
		mmdModel2->draw_light_depth(commList.get());
		mmdModel3->draw_light_depth(commList.get());

		//
		//mmd���؂�|���S���֕`��
		//
		
		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);

		perapolygon->all_resource_barrior(commList.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		perapolygon->clear(commList.get());

		auto [renderNum, perapolygonHandle] = perapolygon->get_render_target_info();
		auto depthCpuHandle = depthBuffer->get_cpu_handle(0);
		commList->get()->OMSetRenderTargets(renderNum, perapolygonHandle, false, &depthCpuHandle);

		depthBuffer->clear(commList.get(), 0);

		mmdModelRenderer.preparation_for_drawing(commList.get());
		mmdModel->draw(commList.get());
		mmdModel2->draw(commList.get());
		mmdModel3->draw(commList.get());


		perapolygon->draw_shrink_texture_for_blur(commList.get());

		perapolygon->all_resource_barrior(commList.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);


		
		//
		//�؂�|���S�����o�b�N�o�b�t�@�ɕ`��
		//

		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);

		//�؂�|���S���̕`�ʂȂ̂Ő[�x�o�b�t�@�͂���Ȃ�
		doubleBuffer->begin_drawing_to_backbuffer(commList.get(), nullptr);
		doubleBuffer->clear_back_buffer(commList.get());

		perapolygon->draw(commList.get());

		doubleBuffer->end_drawing_to_backbuffer(commList.get());

		commList->get()->Close();
		commList->execute();

		commList->clear();

		doubleBuffer->flip();

	}
	
	return 0;
}