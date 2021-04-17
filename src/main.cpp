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


//シャドウマップ用の深度バッファ生成時に使用
constexpr uint32_t shadow_difinition = 1024;

int main()
{
	//usingしないと演算子が使えない
	using namespace DirectX;

	auto hwnd = DX12::create_window(L"directx", window_width, window_height);

	//とりあえずスマートポインタ使っておく
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
	//普通の深度とライトの深度用
	//
	DX12::depth_buffer<2> depthBuffer{};
	if (!depthBuffer.initialize(&device, std::make_pair(window_width, window_height), std::make_pair(1024u, 1024u))) {
		std::cout << "depth is failed\n";
		return false;
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
	auto lightPos = XMLoadFloat3(&target) + XMVector3Normalize(XMLoadFloat3(&parallelLightVec))
		* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];

	auto lightCamera = XMMatrixLookAtLH(lightPos, XMLoadFloat3(&target), XMLoadFloat3(&up))* XMMatrixOrthographicLH(50, 50, -10.f, 100.f);


	//
	//mmdモデルの頂点情報
	//頂点レイアウトとシェーダを変更するので注意
	//


	DX12::mmd_model mmdModel{};
	{
		auto result = MMDL::load_pmx("../../mmd/Paimon/派蒙.pmx");
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
	//ぺらポリゴン
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
	//ライト深度用
	//
	D3D12_VIEWPORT lightDepthViewport{};
	lightDepthViewport.Width = static_cast<float>(shadow_difinition);//出力先の幅(ピクセル数)
	lightDepthViewport.Height = static_cast<float>(shadow_difinition);//出力先の高さ(ピクセル数)
	lightDepthViewport.TopLeftX = 0;//出力先の左上座標X
	lightDepthViewport.TopLeftY = 0;//出力先の左上座標Y
	lightDepthViewport.MaxDepth = 1.0f;//深度最大値
	lightDepthViewport.MinDepth = 0.0f;//深度最小値
	
	D3D12_RECT lightDepthScissorRect{};
	lightDepthScissorRect.top = 0;//切り抜き上座標
	lightDepthScissorRect.left = 0;//切り抜き左座標
	lightDepthScissorRect.right = lightDepthScissorRect.left + shadow_difinition;//切り抜き右座標
	lightDepthScissorRect.bottom = lightDepthScissorRect.top + shadow_difinition;//切り抜き下座標

	mmdModel.rotation_bone(16, DirectX::XMMatrixRotationZ(-DirectX::XM_PIDIV2));

	while (DX12::update_window()) {
		
		mmdModel.map_scene_data({ view,proj,lightCamera, shadow, eye });
		mmdModel.update();

		XMVECTOR det;
		auto invProj = XMMatrixInverse(&det, proj);
		perapolygon.map_scene_data({ view,proj,invProj,lightCamera,shadow,eye });

		//
		//光のディプス描写
		//

		depthBuffer.clear(&commList, 1);

		commList.set_viewport(lightDepthViewport);
		commList.set_scissor_rect(lightDepthScissorRect);

		commList.set_render_target(0, nullptr, depthBuffer.get_cpu_handle(1));

		mmdModelRenderer.preparation_for_drawing_light_depth(&commList);
		mmdModel.draw_light_depth(&commList);

		//
		//mmdをぺらポリゴンへ描写
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
		//SSAO用のデータを描写
		//
		commList.set_viewport(viewport);
		commList.set_scissor_rect(scissorrect);

		perapolygon.ssao_resource_barrior(&commList, D3D12_RESOURCE_STATE_RENDER_TARGET);
		
		commList.set_render_target(perapolygon.get_ssao_cpu_handle());

		perapolygonRenderer.preparation_for_drawing_for_SSAO(&commList);

		perapolygon.draw(&commList);

		perapolygon.ssao_resource_barrior(&commList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//
		//ぺらポリゴンをバックバッファに描写
		//

		commList.set_viewport(viewport);
		commList.set_scissor_rect(scissorrect);

		//ぺらポリゴンの描写なので深度バッファはいらない
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