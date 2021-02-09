#include"window.hpp"
#include"window_size.hpp"
#include"DirectX12/device.hpp"
#include"DirectX12/double_buffer.hpp"
#include"DirectX12/command_list.hpp"
#include"DirectX12/shader.hpp"
#include"DirectX12/vertex_buffer.hpp"
#include"DirectX12/index_buffer.hpp"
#include"DirectX12/constant_buffer_resource.hpp"
#include"DirectX12/descriptor_heap.hpp"
#include"DirectX12/texture_shader_resource.hpp"
#include"include/load_pmx.hpp"
#include<DirectXMath.h>
#include<memory>
#include<array>

#include"mmd_model.hpp"
#include"scene_data.hpp"

#include"perapolygon_pipline_state.hpp"
#include"perapolygon_renderer.hpp"

#include"perapolygon.hpp"


#include<iostream>


//糖衣用
template<typename T,typename... Args>
std::shared_ptr<T> create_shared_ptr(ichi::device* device,Args&&... args) {
	return std::shared_ptr<T>{ device->create<T>(std::forward<Args>(args)...)};
}


int main()
{

	auto hwnd = ichi::create_window(L"aaaaa", window_width, window_height);

	//とりあえずスマートポインタ使っておく
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
	//ぺらポリゴン
	//
	auto perapolygon = std::make_unique<ichi::perapolygon>();
	if (!perapolygon->initialize(device.get())) {
		std::cout << "pera false";
		return 0;
	}
	
	

	while (ichi::update_window()) {
		
		//回転の計算
		worldMat *= DirectX::XMMatrixRotationRollPitchYaw(0.f, 0.01f, 0.f);
	

		mmdModel->map_scene_data({ worldMat,view,proj,lightCamera, shadow, eye });

		//
		//光のディプス描写
		//
		mmdModel->draw_light_depth(commList.get());

		//
		//mmdをぺらポリゴンへ描写
		//
		
		commList->get()->RSSetViewports(1, &viewport);
		commList->get()->RSSetScissorRects(1, &scissorrect);

		perapolygon->begin_drawing_resource_barrier(commList.get());
		perapolygon->clear(commList.get());

		auto [renderNum, perapolygonHandle] = perapolygon->get_render_target_info();
		auto mmdCpuHandle = mmdModel->get_depth_resource_cpu_handle();
		commList->get()->OMSetRenderTargets(renderNum, perapolygonHandle, false, &mmdCpuHandle);

		commList->get()->ClearDepthStencilView(mmdModel->get_depth_resource_cpu_handle(),D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		mmdModel->draw(commList.get());

		perapolygon->end_drawing_resource_barrier(commList.get());

		//
		//ぺらポリゴンをバックバッファに描写
		//

		doubleBuffer->begin_drawing_to_backbuffer(commList.get(), mmdModel->get_depth_resource_cpu_handle());
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