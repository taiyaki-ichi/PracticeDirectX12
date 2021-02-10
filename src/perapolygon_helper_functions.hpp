#pragma once
#include<optional>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<memory>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


namespace ichi
{
	class device;
	class vertex_buffer;

	//�؂�|���S���p���[�g�V�O�l�`���̍쐻
	std::optional<ID3D12RootSignature*> create_perapolygon_root_signature(device*);

	//�؂�|���S���p�̃p�C�v���C���X�e�[�g�̍쐻
	//�����͂؂�|���S���p�̃��[�g�V�O�l�`��
	//�Q�ڂ͂ڂ����p
	std::optional<std::pair<ID3D12PipelineState*, ID3D12PipelineState*>> create_perapolygon_pipline_state(device*, ID3D12RootSignature*);

	//�F�p�̃��\�[�X�Ɩ@���p�̃��\�[�X�̐���
	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>> create_perapolygon_resource(device*);

	//�؂�|���S�����o�b�N�o�b�t�@�ɕ`�ʂ���̃o�[�e�b�N�X�o�b�t�@
	std::optional<std::unique_ptr<vertex_buffer>> create_perapolygon_vertex_buffer(device*);

	//bloom�p��
	//�F�p�Ƃ��̂Ƃقړ����Ȃ̂ł܂Ƃ߂邩��
	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>> create_perapolygon_bloom_resource(device*);


}