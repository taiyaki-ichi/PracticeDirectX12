#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//mmd�̕`�ʂɎg�p����[�x�o�b�t�@�ƃ��C�g�[�x�o�b�t�@�̍쐻
	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>>
		create_mmd_depth_buffers(device*, unsigned int windowWidth, unsigned int windowHeight, unsigned int shadowDifinition);

	

}