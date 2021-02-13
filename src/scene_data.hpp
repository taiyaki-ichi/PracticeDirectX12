#pragma once
#include<DirectXMath.h>

namespace DX12
{
	//しぇだーに渡す用
	struct scene_data
	{
		DirectX::XMMATRIX m_world{};
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};

		//ライト追加
		DirectX::XMMATRIX m_light_camera{};
		//影追加
		DirectX::XMMATRIX m_shadow{};

		DirectX::XMFLOAT3 m_eye{};
	};

}