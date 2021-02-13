#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace DX12
{
	class device;

	//mmd�p�̃��[�g�V�O�l�`���̍쐻
	std::optional<ID3D12RootSignature*> create_mmd_rootsignature(device*);

	//mmd�p�̃p�C�v���C���X�e�[�g�̍쐻
	//�O�҂͕��ʂ́A��҂̓V���h�E�p��
	std::optional<std::pair<ID3D12PipelineState*, ID3D12PipelineState*>> create_mmd_pipline_state(device*, ID3D12RootSignature*);

}