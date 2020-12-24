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
#include<DirectXMath.h>
#include<memory>
#include<array>


#include<iostream>

int main()
{
	constexpr unsigned int window_width = 800;
	constexpr unsigned int window_height = 600;
	auto hwnd = ichi::create_window(L"aaaaa", window_width, window_height);

	//解放めんどいのでとりあえずスマートポインタ使っておく
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

	//シェーダ
	auto vertShaderBlob = ichi::create_shader_blob(L"shader/VertexShader1.hlsl", "main", "vs_5_0");
	auto pixcShaderBlob = ichi::create_shader_blob(L"shader/PixelShader1.hlsl", "main", "ps_5_0");

	auto pipelineState = std::shared_ptr<ichi::pipeline_state>(device->create<ichi::pipeline_state>(vertShaderBlob, pixcShaderBlob));
	if (!pipelineState) {
		std::cout << "pipe is failed\n";
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


	struct Vertex {
		DirectX::XMFLOAT3 pos;//XYZ座標
		DirectX::XMFLOAT2 uv;//UV座標
	};

	Vertex vertices[] = {
		{{-1.f,-1.f,0.0f},{0.0f,1.0f} },//左下
		{{-1.f,1.f,0.0f} ,{0.0f,0.0f}},//左上
		{{1.f,-1.f,0.0f} ,{1.0f,1.0f}},//右下
		{{1.f,1.f,0.0f} ,{1.0f,0.0f}},//右上
	};
	auto vertexBuffer = std::shared_ptr<ichi::vertex_buffer>{ device->create<ichi::vertex_buffer>(sizeof(vertices), sizeof(vertices[0])) };
	if (!vertexBuffer) {
		std::cout << "vert buffer is failed\n";
		return 0;
	}
	vertexBuffer->map(vertices);

	//インデックス情報
	unsigned short indices[] = { 0,1,2, 2,1,3 };
	auto indexBuffer = std::shared_ptr<ichi::index_buffer>{ device->create<ichi::index_buffer>(sizeof(indices)) };
	if (!indexBuffer) {
		std::cout << "index buff is failed\n";
		return 0;
	}
	indexBuffer->map(indices);

	//定数バッファ
	DirectX::XMFLOAT3 eye{ 0,0,5 };
	DirectX::XMFLOAT3 target{ 0,0,0 };
	DirectX::XMFLOAT3 up{ 0,1,0 };
	auto pos = DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&eye), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
	pos *= DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV2,
		static_cast<float>(window_width) / static_cast<float>(window_height),
		1.f,
		10.f
		);
	auto constantBuffer = std::shared_ptr<ichi::constant_buffer_resource>{
		device->create<ichi::constant_buffer_resource>(sizeof(decltype(pos)))
	};

	constantBuffer->map(pos);
	
	//定数バッファとシェーダリソース用のディスクリプタヒープ
	auto bufferDescriptorHeap = std::shared_ptr<ichi::descriptor_heap>{
		device->create<ichi::descriptor_heap>(ichi::DESCRIPTOR_HEAP_SIZE)
	};

	bufferDescriptorHeap->create_view(device.get(), constantBuffer.get());

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

	while (ichi::update_window()) {

		doubleBuffer->begin_drawing_to_backbuffer(commList.get());

		//コマンドリストのメンバにまとめてしまうか
		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);

		commList->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commList->get()->IASetVertexBuffers(0, 1, &vertexBuffer->get_view());
		commList->get()->IASetIndexBuffer(&indexBuffer->get_view());

		commList->get()->SetPipelineState(pipelineState->get());
		commList->get()->SetGraphicsRootSignature(pipelineState->get_root_signature());

		commList->get()->SetDescriptorHeaps(1, &bufferDescriptorHeap->get());
		//commList->get()->SetDescriptorHeaps(2, descriptorHeaps);
		commList->get()->SetGraphicsRootDescriptorTable(0, bufferDescriptorHeap->get()->GetGPUDescriptorHandleForHeapStart());
		//commList->get()->SetGraphicsRootDescriptorTable(0, descriptorHeaps[1]->GetGPUDescriptorHandleForHeapStart());

		commList->get()->DrawIndexedInstanced(6, 1, 0, 0, 0);

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