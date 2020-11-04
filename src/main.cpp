#include"window.hpp"
#include<iostream>
#include"DirectX12/init.hpp"
#include"DirectX12/buffer.hpp"
#include"DirectX12/draw.hpp"
#include"DirectX12/shader.hpp"

int main()
{

	/*
	using namespace DirectX;

	//�Ȃ��Ɠ�������
	auto hoge = CoInitializeEx(0, COINIT_MULTITHREADED);

	TexMetadata meta{};
	ScratchImage scratch{};

	auto result = LoadFromWICFile(L"../texture/icon.png", WIC_FLAGS_NONE, &meta, scratch);

	std::cout << "result : " << result << "\n";

	if (SUCCEEDED(result))
		std::cout << meta.depth*meta.width*meta.height << "\n" << scratch.GetPixelsSize() << "\n" << scratch.GetImage(0, 0, 0)->slicePitch << "\n";
		*/
	
	const wchar_t* WINDOW_NAME = L"aaa";

	constexpr unsigned int WINDOW_WIDTH = 400;
	constexpr unsigned int WINDOW_HEIGHT = 300;

	auto hwnd = graphics::create_window(WINDOW_NAME,WINDOW_WIDTH, WINDOW_HEIGHT);

	using namespace graphics;

	init_debug();
	auto factory = create_dxgi_factory();
	auto adapter = get_adapter(factory);
	auto device = create_device(adapter);
	auto commandAllocator = create_command_allocator(device);
	auto commandList = create_command_list(device, commandAllocator);
	auto commandQueue = create_command_queue(device, commandList);
	auto swapChain = create_swap_chain(factory, commandQueue, hwnd);
	auto descriptorHeap = create_descriptor_heap(device);
	auto buffers = create_double_buffer(device, swapChain, descriptorHeap);
	auto [fence, fenceVal] = create_fence(device);

	struct Vertex {
		DirectX::XMFLOAT3 pos;//XYZ���W
		DirectX::XMFLOAT2 uv;//UV���W
	};

	Vertex vertices[] = {
		{{-0.4f,-0.7f,0.0f},{0.0f,1.0f} },//����
		{{-0.4f,0.7f,0.0f} ,{0.0f,0.0f}},//����
		{{0.4f,-0.7f,0.0f} ,{1.0f,1.0f}},//�E��
		{{0.4f,0.7f,0.0f} ,{1.0f,0.0f}},//�E��
	};

	auto vertBuffer = create_buffer(device, sizeof(vertices));
	map(vertBuffer, vertices);
	auto vertBufferView = get_vertex_buffer_view(vertBuffer, sizeof(vertices), sizeof(vertices[0]));

	//�C���f�b�N�X���
	unsigned short indices[] = { 0,1,2, 2,1,3 };
	auto indexBuffer = create_buffer(device, sizeof(indices));
	map(indexBuffer, indices);
	auto indexBufferView = get_index_buffer_view(indexBuffer, sizeof(indices));

	//�V�F�[�_
	auto vertShaderBlob = create_shader_blob(L"shader/VertexShader1.hlsl", "main","vs_5_0");
	auto pixcShaderBlob = create_shader_blob(L"shader/PixelShader1.hlsl", "main","ps_5_0");

	//���[�g�V�O�l�`��
	auto rootSignature = create_root_signature(device);

	//�O���t�B�N�X�p�C�v���C��
	auto graphicsPipeline = create_graphics_pipline_state(device, vertShaderBlob, pixcShaderBlob, rootSignature);

	//�r���[�|�[�g
	auto viewPort = get_viewport(WINDOW_WIDTH, WINDOW_HEIGHT);

	//�V�U�[��`
	auto scissorRect = get_scissor_rect(WINDOW_WIDTH, WINDOW_HEIGHT);

	//
	//�e�N�X�`��
	//

	//�Ȃ��Ɠ����Ȃ�����
	auto hoge = CoInitializeEx(0, COINIT_MULTITHREADED);

	DirectX::TexMetadata metaData{};
	DirectX::ScratchImage scratch{};

	auto result = LoadFromWICFile(L"../texture/icon.png", DirectX::WIC_FLAGS_NONE, &metaData, scratch);
	auto image = scratch.GetImage(0, 0, 0);

	//���\�[�X�m��
	auto uploadBuffer = create_texture_unload_buffer(device, image);
	auto textureBuffer = create_texture_buffer(device, metaData);

	uploadBuffer->GetDesc();
	textureBuffer->GetDesc();

	//�e�N�X�`�����}�b�v
	map(uploadBuffer, image);
	
	//location�擾
	auto [uploadLocation, textureLocation] = get_texture_copy_location(device, uploadBuffer, textureBuffer, metaData, image);

	//�R�}���h���s
	copy_texture(commandAllocator, commandList, commandQueue, fence, fenceVal, uploadLocation, textureLocation);

	//
	//��
	//

	ID3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NodeMask = 0;//�}�X�N��0
	descHeapDesc.NumDescriptors = 1;//�r���[�͍��̂Ƃ���P����
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�V�F�[�_���\�[�X�r���[(����ђ萔�AUAV��)
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));//����

	//�ʏ�e�N�X�`���r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = metaData.format;//DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA(0.0f�`1.0f�ɐ��K��)
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//��q
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

	device->CreateShaderResourceView(textureBuffer, //�r���[�Ɗ֘A�t����o�b�t�@
		&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
		texDescHeap->GetCPUDescriptorHandleForHeapStart()//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
	);
	
	//
	//
	//


	while (graphics::process_window_message())
	{

		auto bbIdx = swapChain->GetCurrentBackBufferIndex();

		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = buffers[bbIdx];
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		commandList->ResourceBarrier(1, &BarrierDesc);

		commandList->SetPipelineState(graphicsPipeline);

		//�����_�[�^�[�Q�b�g���w��
		auto rtvH = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		commandList->OMSetRenderTargets(1, &rtvH, false, nullptr);

		//��ʃN���A
		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//���F
		commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		//���낢��
		commandList->RSSetViewports(1, &viewPort);
		commandList->RSSetScissorRects(1, &scissorRect);
		commandList->SetGraphicsRootSignature(rootSignature);

		//Bufferview
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);

		commandList->SetGraphicsRootSignature(rootSignature);
		commandList->SetDescriptorHeaps(1, &texDescHeap);
		commandList->SetGraphicsRootDescriptorTable(0, texDescHeap->GetGPUDescriptorHandleForHeapStart());

		//�C���f�b�N�X�ŕ`��
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		commandList->ResourceBarrier(1, &BarrierDesc);

		//���߂̃N���[�Y
		commandList->Close();


		//�R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, cmdlists);
		////�҂�
		commandQueue->Signal(fence, ++fenceVal);

		if (fence->GetCompletedValue() != fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		commandAllocator->Reset();//�L���[���N���A
		commandList->Reset(commandAllocator, nullptr);//�ĂуR�}���h���X�g�����߂鏀��


		//�t���b�v
		swapChain->Present(1, 0);
	}
	
	
	UnregisterClass(WINDOW_NAME, GetModuleHandle(nullptr));

	//factory->Release();
	//adapter->Release();

	device->Release();
	commandAllocator->Release();
	commandList->Release();
	commandQueue->Release();
	swapChain->Release();
	descriptorHeap->Release();
	for (auto b : buffers)
		b->Release();
	fence->Release();

	vertBuffer->Release();
	indexBuffer->Release();

	vertShaderBlob->Release();
	pixcShaderBlob->Release();

	rootSignature->Release();

	graphicsPipeline->Release();

	
	
	return 0;
}