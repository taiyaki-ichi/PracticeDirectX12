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

#include"perapolygon_pipline_state.hpp"
#include"perapolygon_renderer.hpp"


#include<iostream>


//糖衣用
template<typename T,typename... Args>
std::shared_ptr<T> create_shared_ptr(ichi::device* device,Args&&... args) {
	return std::shared_ptr<T>{ device->create<T>(std::forward<Args>(args)...)};
}


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

	auto commList = create_shared_ptr<ichi::command_list>(device.get());
	if (!commList) {
		std::cout << "comList is failed\n";
		return 0;
	}

	auto doubleBuffer = create_shared_ptr<ichi::double_buffer>(device.get(), hwnd, commList.get());
	if (!doubleBuffer) {
		std::cout << "douebl is failed\n";
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

	//
	//viewproj
	//
	DirectX::XMFLOAT3 eye{ 0,6,-6 };
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
	//モデル本体を回転させたり移動させたりする行列
	//
	DirectX::XMMATRIX worldMat = DirectX::XMMatrixIdentity();


	//平行ライトの向き
	//右下奥向き
	DirectX::XMFLOAT3 parallelLightVec{ 1.f,-1.f,1.f };
	//一括の方法が分からぬ
	parallelLightVec.x *= -1.f;
	parallelLightVec.y *= -1.f;
	parallelLightVec.z *= -1.f;
	//影がおちる平面の方程式のデータ
	DirectX::XMFLOAT4 planeVec{ 0.f,1.f,0.f,0.f };
	//実際の影行列
	DirectX::XMMATRIX shadow = DirectX::XMMatrixShadow(
		DirectX::XMLoadFloat4(&planeVec), DirectX::XMLoadFloat3(&parallelLightVec));

	//ライトのポジション
	//usingしないと演算子が使えない
	using namespace DirectX;
	auto lightPos = XMLoadFloat3(&target) + XMVector3Normalize(XMLoadFloat3(&parallelLightVec))
		* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];

	auto lightCamera = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up))* XMMatrixOrthographicLH(50, 50, 1.f, 100.f);


	//
	//mmdモデルの頂点情報
	//頂点レイアウトとシェーダを変更するので注意
	//
	//モデルの読み込み
	//
	auto mmdModel = std::shared_ptr<ichi::mmd_model>{};

	auto modelIf = MMDL::load_pmx("../../mmd/Paimon/派蒙.pmx");
	if (modelIf)
	{
		//とりあえず
		auto& model = std::get<MMDL::pmx_model<std::wstring>>(modelIf.value());

		mmdModel = create_shared_ptr<ichi::mmd_model>(device.get(), model, commList.get());
	}
	else {
		std::cout << "model loaf failed\n";
		return 0;
	}

	//
	//ディプス
	//
	auto depthBuffer = create_shared_ptr<ichi::depth_buffer>(device.get(), window_width, window_height);

	
	//
	//
	//
	//ぺらポリごん用のシェーダ
	auto peraVertShaderBlob = ichi::create_shader_blob(L"shader/peraVertexShader.hlsl", "main", "vs_5_0");
	auto peraPixcShaderBlob = ichi::create_shader_blob(L"shader/peraPixelShader.hlsl", "main", "ps_5_0");

	//ぺら用パイプライン
	auto peraPipelineState = std::make_shared<ichi::perapolygon_pipline_state>();
	if (!peraPipelineState->initialize(device.get(),peraVertShaderBlob,peraPixcShaderBlob)) {
		return 0;
	}

	//
	auto peraRenderer = std::make_shared<ichi::perapolygon_renderer>();
	if (!peraRenderer->initialize(device.get())) {
		return 0;
	}

	//ぺら用頂点バッファ
	auto peraVertexBuff = std::make_shared<ichi::vertex_buffer>();
	{
		struct PeraVert{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 uv;
		};
		PeraVert pv[4] = {
			{{-1.f,-1.f,0.1f},{0,1.f}},
			{{-1.f,1.f,0.1f},{0,0}},
			{{1.f,-1.f,0.1f},{1.f,1.f}},
			{{1.f,1.f,0.1f},{1.f,0}},
		};

		if (!peraVertexBuff->initialize(device.get(), sizeof(pv), sizeof(pv[0]))) {
			return 0;
		}
		peraVertexBuff->map(pv);
	}

	//ぺら用ディスクリプタヒープ
	//ぺらポリゴンを描写するときに使う
	auto peraDescriptorHeap = std::make_shared<ichi::descriptor_heap<ichi::descriptor_heap_type::CBV_SRV_UAV>>();
	//とりあえず大きさは1
	if (!peraDescriptorHeap->initialize(device.get(), 1)) {
		return 0;
	}
	auto peraDescriptorHeapHandle = peraDescriptorHeap->create_view(device.get(), peraRenderer.get());
	if (!peraDescriptorHeapHandle) {
		std::cout << "pera handle is failed\n";
		return 0;
	}
	
	//
	//
	//

	while (ichi::update_window()) {
		
		//回転の計算
		worldMat *= DirectX::XMMatrixRotationRollPitchYaw(0.f, 0.01f, 0.f);
	

		mmdModel->map_scene_data({ worldMat,view,proj,lightCamera, shadow, eye });

	


		mmdModel->draw_light_depth(commList.get());

		//
		//mmdをぺらポリゴンへ描写
		//
		
		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);

		peraRenderer->begin_drawing(commList.get(), depthBuffer.get());

		depthBuffer->clear(commList.get());
		peraRenderer->clear(commList.get());

		mmdModel->draw(commList.get());

		peraRenderer->end_drawing(commList.get());

		//
		//
		//

		//ぺらポリゴンをバックバッファに描写
		
		doubleBuffer->begin_drawing_to_backbuffer(commList.get(), depthBuffer.get());
		doubleBuffer->clear_back_buffer(commList.get());

		depthBuffer->clear(commList.get());

		commList->get()->SetPipelineState(peraPipelineState->get());
		commList->get()->SetGraphicsRootSignature(peraPipelineState->get_root_signature());

		commList->get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		commList->get()->IASetVertexBuffers(0, 1, &peraVertexBuff->get_view());
		commList->get()->SetDescriptorHeaps(1, &peraDescriptorHeap->get());
		commList->get()->SetGraphicsRootDescriptorTable(0, peraDescriptorHeap->get()->GetGPUDescriptorHandleForHeapStart());
		commList->get()->DrawInstanced(4, 1, 0, 0);

		doubleBuffer->end_drawing_to_backbuffer(commList.get());

		commList->get()->Close();
		commList->execute();

		commList->clear();

		doubleBuffer->flip();

	}
	
	return 0;
}