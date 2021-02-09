#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace ichi
{
	class device;

	//mmd�p�̃��[�g�V�O�l�`���̍쐻
	std::optional<ID3D12RootSignature*> create_mmd_rootsignature(device*);

	//mmd�p�̃p�C�v���C���X�e�[�g�̍쐻
	//�O�҂͕��ʂ́A��҂̓V���h�E�p��
	std::optional<std::pair<ID3D12PipelineState*, ID3D12PipelineState*>> create_mmd_pipline_state(device*, ID3D12RootSignature*);

	//mmd�̕`�ʂɎg�p����[�x�o�b�t�@�ƃ��C�g�[�x�o�b�t�@�̍쐻
	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>>
		create_mmd_depth_buffers(device*, unsigned int windowWidth, unsigned int windowHeight, unsigned int shadowDifinition);

}