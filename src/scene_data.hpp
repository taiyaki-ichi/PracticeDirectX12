#pragma once
#include<DirectXMath.h>

namespace ichi
{
	//しぇだーに渡す用
	struct scene_data
	{
		DirectX::XMMATRIX m_world{};
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};
		DirectX::XMFLOAT3 m_eye{};
	};

}