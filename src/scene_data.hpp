#pragma once
#include<DirectXMath.h>

namespace DX12
{
	//mmdmodel�ɓn���p
	struct scene_data
	{
		DirectX::XMMATRIX m_world{};
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};

		//���C�g�ǉ�
		DirectX::XMMATRIX m_light_camera{};
		//�e�ǉ�
		DirectX::XMMATRIX m_shadow{};

		DirectX::XMFLOAT3 m_eye{};
	};

	//perapolygon�ɓn���p
	struct scene_data_for_perapolygon
	{
		
		DirectX::XMMATRIX m_view{};
		DirectX::XMMATRIX m_proj{};
		DirectX::XMMATRIX m_inv_proj{};

		//���C�g�ǉ�
		DirectX::XMMATRIX m_light_camera{};
		//�e�ǉ�
		DirectX::XMMATRIX m_shadow{};

		DirectX::XMFLOAT3 m_eye{};
	};

}