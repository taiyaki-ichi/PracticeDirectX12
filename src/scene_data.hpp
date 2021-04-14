#pragma once
#include<DirectXMath.h>
#include<array>

namespace DX12
{
	//mmdmodel‚É“n‚·—p
	struct scene_data
	{
		//DirectX::XMMATRIX m_world{};
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};

		//ƒ‰ƒCƒg’Ç‰Á
		DirectX::XMMATRIX m_light_camera{};
		//‰e’Ç‰Á
		DirectX::XMMATRIX m_shadow{};

		DirectX::XMFLOAT3 m_eye{};
	};

	struct transform_data 
	{
		DirectX::XMMATRIX m_world{};
		std::array<DirectX::XMMATRIX, 512> m_bones{};
	};

	//perapolygon‚É“n‚·—p
	struct scene_data_for_perapolygon
	{
		
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};
		DirectX::XMMATRIX m_inv_proj{};

		//ƒ‰ƒCƒg’Ç‰Á
		DirectX::XMMATRIX m_light_camera{};
		//‰e’Ç‰Á
		DirectX::XMMATRIX m_shadow{};

		DirectX::XMFLOAT3 m_eye{};
	};


	
}