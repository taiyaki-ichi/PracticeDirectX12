#pragma once
#include<DirectXMath.h>

namespace ichi
{
	//�������[�ɓn���p
	struct scene_data
	{
		DirectX::XMMATRIX m_world{};
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};
		DirectX::XMFLOAT3 m_eye{};
	};

}