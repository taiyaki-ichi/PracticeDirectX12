#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;
	class command_list;

	class mmd_model_renderer
	{
		//�ʏ�̃p�C�v���C���X�e�[�g
		ID3D12PipelineState* m_pipeline_state = nullptr;
		//�V���h�E�p�̃p�C�v���C���X�e�[�g
		ID3D12PipelineState* m_light_depth_pipeline_state = nullptr;
		//���[�g�V�O�l�`��
		ID3D12RootSignature* m_root_signature = nullptr;

	public:
		mmd_model_renderer() = default;
		~mmd_model_renderer();

		bool initialize(device*);

		//mmd_model��`�ʂ���O�ɌĂяo��
		void preparation_for_drawing(command_list*);
		
		//���C�g�[�x��`�ʂ���O�ɌĂяo��
		void preparation_for_drawing_light_depth(command_list*);


		//�R�s�[�֎~
		mmd_model_renderer(const mmd_model_renderer&) = delete;
		mmd_model_renderer& operator=(const mmd_model_renderer&) = delete;
		//���[�u�\
		mmd_model_renderer(mmd_model_renderer&&) noexcept;
		mmd_model_renderer& operator=(mmd_model_renderer&&) noexcept;
	};

}