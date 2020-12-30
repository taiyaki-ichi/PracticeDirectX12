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
#include"my_vertex.hpp"
#include"DirectX12/depth_buffer.hpp"
#include<DirectXMath.h>
#include<memory>
#include<array>



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

	/*

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
	auto vertexBuffer = std::shared_ptr<ichi::vertex_buffer>{ device->create<ichi::vertex_buffer>(sizeof(vertices), sizeof(vertices[0])) };
	if (!vertexBuffer) {
		std::cout << "vert buffer is failed\n";
		return 0;
	}
	vertexBuffer->map(vertices);
	*/

	/*

	//�C���f�b�N�X���
	unsigned short indices[] = { 0,1,2, 2,1,3 };
	auto indexBuffer = std::shared_ptr<ichi::index_buffer>{ device->create<ichi::index_buffer>(sizeof(indices)) };
	if (!indexBuffer) {
		std::cout << "index buff is failed\n";
		return 0;
	}
	indexBuffer->map(indices);
	*/

	//�萔�o�b�t�@
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
	auto viewproj = view * proj;
	auto viewprojConstantBuffer = std::shared_ptr<ichi::constant_buffer_resource>{
		device->create<ichi::constant_buffer_resource>(sizeof(DirectX::XMMATRIX))
	};
	viewprojConstantBuffer->map(viewproj);

	//
	//���f���{�̂���]��������ړ��������肷��s��
	//
	DirectX::XMMATRIX worldMat = DirectX::XMMatrixIdentity();
	auto worldConstantBuffer = std::shared_ptr<ichi::constant_buffer_resource>{
		device->create<ichi::constant_buffer_resource>(sizeof(DirectX::XMMATRIX))
	};
	worldConstantBuffer->map(worldMat);

	//
	//�萔�o�b�t�@�ƃV�F�[�_���\�[�X�p�̃f�B�X�N���v�^�q�[�v
	//
	auto constantBufferDescriptorHeap = std::shared_ptr<ichi::descriptor_heap>{
		device->create<ichi::descriptor_heap>(ichi::DESCRIPTOR_HEAP_SIZE)
	};

	//b�̃��W�X�^0��world�s��A1��view��proj�̊|�����킹
	constantBufferDescriptorHeap->create_view(device.get(), worldConstantBuffer.get());
	constantBufferDescriptorHeap->create_view(device.get(), viewprojConstantBuffer.get());


	/*
	auto imageResult = ichi::get_texture(L"../texture/icon.png");
	if (!imageResult) {
		std::cout << "image si failed\n";
		return 0;
	}
	auto& [metaData, scratchImage] = imageResult.value();

	auto textureBuffer = std::shared_ptr<ichi::texture_shader_resource>{
		device->create<ichi::texture_shader_resource>(&metaData,&scratchImage)
	};
	auto uploadTextureBuffer = std::shared_ptr<ichi::upload_texture_shader_resource>{
		device->create<ichi::upload_texture_shader_resource>(&metaData,&scratchImage)
	};

	uploadTextureBuffer->map(*scratchImage.GetImage(0, 0, 0));

	commList->copy_texture(uploadTextureBuffer.get(), textureBuffer.get());
	commList->get()->Close();
	commList->execute();
	commList->clear();

	bufferDescriptorHeap->create_view(device.get(), textureBuffer.get());

	*/

	//
	//mmd���f���̒��_���
	//���_���C�A�E�g�ƃV�F�[�_��ύX����̂Œ���
	//
	//���f���̓ǂݍ���
	//
	std::vector<ichi::my_vertex> mmdVertex{};
	std::vector<ichi::my_surface> mmdSurface{};
	std::vector<ichi::my_material> mmdMaterial{};
	auto modelIf = MMDL::load_pmx("../../mmd/Paimon/�h��.pmx");
	if (modelIf)
	{
		auto&& model = modelIf.value();

		std::visit([&mmdVertex](auto& m) {
			mmdVertex = ichi::generate_my_vertex(m.m_vertex);
			}, model);

		std::visit([&mmdSurface](auto& m) {
			for (auto& tmp : m.m_surface)
				mmdSurface.emplace_back(ichi::my_surface{ static_cast<unsigned short>(tmp.m_vertex_index) });
			}, model);

		std::visit([&mmdMaterial](auto& m) {
			mmdMaterial = ichi::generate_my_material
				<typename std::remove_reference_t<decltype(m)>::string_type>(m.m_material);
			}, model);
	}
	else {
		std::cout << "model loaf failed\n";
		return 0;
	}

	//
	//���_
	//
	//�v�f���~�v�f������̃T�C�Y�A���Ⴀ�Ȃ��Ɠ����Ȃ�����
	auto mmdVertexBuffer = std::shared_ptr<ichi::vertex_buffer>{ device->create<ichi::vertex_buffer>(mmdVertex.size() * sizeof(mmdVertex[0]), sizeof(mmdVertex[0])) };
	if (!mmdVertexBuffer) {
		std::cout << "vert buffer is failed\n";
		return 0;
	}
	mmdVertexBuffer->map(mmdVertex);

	//
	//�C���f�b�N�X
	//
	auto mmdIndexBuffer = std::shared_ptr<ichi::index_buffer>{ device->create<ichi::index_buffer>(mmdSurface.size() * sizeof(mmdSurface[0])) };
	if (!mmdIndexBuffer) {
		std::cout << "index buffer is failed\n";
		return 0;
	}
	mmdIndexBuffer->map(mmdSurface);


	//
	//�}�e���A��
	//
	auto mmdMaterialBuffer = std::shared_ptr<ichi::constant_buffer_resource>{
		device->create<ichi::constant_buffer_resource>(mmdMaterial.size() * sizeof(mmdMaterial[0]))
	};
	mmdMaterialBuffer->map(mmdMaterial);
	constantBufferDescriptorHeap->create_view(device.get(), mmdMaterialBuffer.get());


	//
	//�ł��Ղ�
	//
	auto depthBuffer = std::shared_ptr<ichi::depth_buffer>{
		device->create<ichi::depth_buffer>(window_width,window_height)
	};

	//��]�p
	float rot = 0.f;
	constexpr float radius = 6.f;

	auto hogeConstantBuffer = std::shared_ptr<ichi::constant_buffer_resource>{
		device->create<ichi::constant_buffer_resource>(sizeof(DirectX::XMMATRIX))
	};
	DirectX::XMMATRIX hogeMat = DirectX::XMMatrixIdentity();
	hogeConstantBuffer->map(hogeMat);

	while (ichi::update_window()) {
		
		//��]�̌v�Z
		worldMat *= DirectX::XMMatrixRotationRollPitchYaw(0.f, 0.01f, 0.f);
		worldConstantBuffer->map(worldMat);

		//
		constantBufferDescriptorHeap->reset();
		constantBufferDescriptorHeap->create_view(device.get(), worldConstantBuffer.get());
		constantBufferDescriptorHeap->create_view(device.get(), viewprojConstantBuffer.get());
		constantBufferDescriptorHeap->create_view(device.get(), mmdMaterialBuffer.get());
		//

		doubleBuffer->begin_drawing_to_backbuffer(commList.get(), depthBuffer.get());
		doubleBuffer->clear_back_buffer(commList.get());
		depthBuffer->clear(commList.get());

		//�R�}���h���X�g�̃����o�ɂ܂Ƃ߂Ă��܂���
		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);

		commList->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//commList->get()->IASetVertexBuffers(0, 1, &vertexBuffer->get_view());
		//commList->get()->IASetIndexBuffer(&indexBuffer->get_view());

		commList->get()->IASetVertexBuffers(0, 1, &mmdVertexBuffer->get_view());
		commList->get()->IASetIndexBuffer(&mmdIndexBuffer->get_view());

		commList->get()->SetPipelineState(pipelineState->get());
		commList->get()->SetGraphicsRootSignature(pipelineState->get_root_signature());

		commList->get()->SetDescriptorHeaps(1, &constantBufferDescriptorHeap->get());
		//
		commList->get()->SetGraphicsRootDescriptorTable(0, constantBufferDescriptorHeap->get()->GetGPUDescriptorHandleForHeapStart());

		//commList->get()->DrawIndexedInstanced(6, 1, 0, 0, 0);
		//commList->get()->DrawInstanced(mmdVertex.size(), 1, 0, 0);

		commList->get()->DrawIndexedInstanced(mmdSurface.size(), 1, 0, 0, 0);


		doubleBuffer->end_drawing_to_backbuffer(commList.get());
		commList->get()->Close();

		commList->execute();

		commList->clear(pipelineState.get());




		//
		//
		doubleBuffer->begin_drawing_to_backbuffer(commList.get(), depthBuffer.get());
		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);
		commList->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commList->get()->IASetVertexBuffers(0, 1, &mmdVertexBuffer->get_view());
		commList->get()->IASetIndexBuffer(&mmdIndexBuffer->get_view());

		commList->get()->SetPipelineState(pipelineState->get());
		commList->get()->SetGraphicsRootSignature(pipelineState->get_root_signature());

		constantBufferDescriptorHeap->reset();
		constantBufferDescriptorHeap->create_view(device.get(), hogeConstantBuffer.get());
		constantBufferDescriptorHeap->create_view(device.get(), viewprojConstantBuffer.get());
		constantBufferDescriptorHeap->create_view(device.get(), mmdMaterialBuffer.get());

		commList->get()->SetDescriptorHeaps(1, &constantBufferDescriptorHeap->get());
		commList->get()->SetGraphicsRootDescriptorTable(0, constantBufferDescriptorHeap->get()->GetGPUDescriptorHandleForHeapStart());
		commList->get()->DrawIndexedInstanced(mmdSurface.size(), 1, 0, 0, 0);

		doubleBuffer->end_drawing_to_backbuffer(commList.get());
		commList->get()->Close();
		commList->execute();
		commList->clear(pipelineState.get());
		//
		//



		doubleBuffer->flip();

	}


	vertShaderBlob->Release();
	pixcShaderBlob->Release();

	return 0;
}