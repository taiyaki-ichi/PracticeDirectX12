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

	//ないと動かった
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
		DirectX::XMFLOAT3 pos;//XYZ座標
		DirectX::XMFLOAT2 uv;//UV座標
	};

	Vertex vertices[] = {
		{{-0.4f,-0.7f,0.0f},{0.0f,1.0f} },//左下
		{{-0.4f,0.7f,0.0f} ,{0.0f,0.0f}},//左上
		{{0.4f,-0.7f,0.0f} ,{1.0f,1.0f}},//右下
		{{0.4f,0.7f,0.0f} ,{1.0f,0.0f}},//右上
	};

	auto vertBuffer = create_buffer(device, sizeof(vertices));
	map(vertBuffer, vertices);
	auto vertBufferView = get_vertex_buffer_view(vertBuffer, sizeof(vertices), sizeof(vertices[0]));

	//インデックス情報
	unsigned short indices[] = { 0,1,2, 2,1,3 };
	auto indexBuffer = create_buffer(device, sizeof(indices));
	map(indexBuffer, indices);
	auto indexBufferView = get_index_buffer_view(indexBuffer, sizeof(indices));

	//シェーダ
	auto vertShaderBlob = create_shader_blob(L"shader/VertexShader1.hlsl", "main","vs_5_0");
	auto pixcShaderBlob = create_shader_blob(L"shader/PixelShader1.hlsl", "main","ps_5_0");

	//ルートシグネチャ
	auto rootSignature = create_root_signature(device);

	//グラフィクスパイプライン
	auto graphicsPipeline = create_graphics_pipline_state(device, vertShaderBlob, pixcShaderBlob, rootSignature);

	//ビューポート
	auto viewPort = get_viewport(WINDOW_WIDTH, WINDOW_HEIGHT);

	//シザー矩形
	auto scissorRect = get_scissor_rect(WINDOW_WIDTH, WINDOW_HEIGHT);

	//
	//テクスチャ
	//

	//ないと動かなかった
	auto hoge = CoInitializeEx(0, COINIT_MULTITHREADED);

	DirectX::TexMetadata metaData{};
	DirectX::ScratchImage scratch{};

	auto result = LoadFromWICFile(L"../texture/icon.png", DirectX::WIC_FLAGS_NONE, &metaData, scratch);
	auto image = scratch.GetImage(0, 0, 0);

	//リソース確保
	auto uploadBuffer = create_texture_unload_buffer(device, image);
	auto textureBuffer = create_texture_buffer(device, metaData);

	uploadBuffer->GetDesc();
	textureBuffer->GetDesc();

	//テクスチャをマップ
	map(uploadBuffer, image);
	
	//location取得
	auto [uploadLocation, textureLocation] = get_texture_copy_location(device, uploadBuffer, textureBuffer, metaData, image);

	//コマンド実行
	copy_texture(commandAllocator, commandList, commandQueue, fence, fenceVal, uploadLocation, textureLocation);

	//
	//仮
	//

	ID3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NodeMask = 0;//マスクは0
	descHeapDesc.NumDescriptors = 1;//ビューは今のところ１つだけ
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//シェーダリソースビュー(および定数、UAVも)
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));//生成

	//通常テクスチャビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = metaData.format;//DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA(0.0f～1.0fに正規化)
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//後述
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

	device->CreateShaderResourceView(textureBuffer, //ビューと関連付けるバッファ
		&srvDesc, //先ほど設定したテクスチャ設定情報
		texDescHeap->GetCPUDescriptorHandleForHeapStart()//ヒープのどこに割り当てるか
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

		//レンダーターゲットを指定
		auto rtvH = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		commandList->OMSetRenderTargets(1, &rtvH, false, nullptr);

		//画面クリア
		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//黄色
		commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		//いろいろ
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

		//インデックスで描写
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		commandList->ResourceBarrier(1, &BarrierDesc);

		//命令のクローズ
		commandList->Close();


		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, cmdlists);
		////待ち
		commandQueue->Signal(fence, ++fenceVal);

		if (fence->GetCompletedValue() != fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		commandAllocator->Reset();//キューをクリア
		commandList->Reset(commandAllocator, nullptr);//再びコマンドリストをためる準備


		//フリップ
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