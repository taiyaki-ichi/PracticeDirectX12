#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<memory>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace DX12
{
	class device;
	class vertex_buffer;

	//�؂�|���S���p���[�g�V�O�l�`���̍쐻
	std::optional<ID3D12RootSignature*> create_perapolygon_root_signature(device*);

	//�؂�|���S���p�̃p�C�v���C���X�e�[�g�̍쐻
	//�����͂؂�|���S���p�̃��[�g�V�O�l�`��
	//�Q�ڂ͂ڂ����p
	std::optional<std::pair<ID3D12PipelineState*, ID3D12PipelineState*>> create_perapolygon_pipline_state(device*, ID3D12RootSignature*);

}