#pragma once
#include<DirectXMath.h>
#include<array>

namespace DX12
{
	//mmdmodelに渡す用
	struct scene_data
	{
		//DirectX::XMMATRIX m_world{};
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};

		//ライト追加
		DirectX::XMMATRIX m_light_camera{};
		//影追加
		DirectX::XMMATRIX m_shadow{};

		DirectX::XMFLOAT3 m_eye{};
	};

	struct transform_data 
	{
		DirectX::XMMATRIX m_world{};
		std::array<DirectX::XMMATRIX, 256> m_bones{};
	};

	//perapolygonに渡す用
	struct scene_data_for_perapolygon
	{
		
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};
		DirectX::XMMATRIX m_inv_proj{};

		//ライト追加
		DirectX::XMMATRIX m_light_camera{};
		//影追加
		DirectX::XMMATRIX m_shadow{};

		DirectX::XMFLOAT3 m_eye{};
	};


	
}