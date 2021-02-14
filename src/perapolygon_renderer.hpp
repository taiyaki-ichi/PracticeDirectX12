#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;
	class command_list;

	class perapolygon_renderer
	{

		ID3D12RootSignature* m_root_signature = nullptr;
		//�ʏ�̃p�C�v���C��
		ID3D12PipelineState* m_pipeline_state = nullptr;
		//�ڂ����p�̃p�C�v���C��
		ID3D12PipelineState* m_blur_pipeline_state = nullptr;
		//SSAO�p�̃p�C�v���C��
		ID3D12PipelineState* m_SSAO_pipeline_state = nullptr;

	public:
		perapolygon_renderer() = default;
		~perapolygon_renderer();

		bool initialize(device*);

		//�؂�|���S���̕`�ʑO�ɌĂяo��
		void preparation_for_drawing(command_list*);

		//�؂�|���S�����ڂ������߂̏k�����ꂽ�o�b�t�@�ɕ`�ʂ���O�Ɏg�p
		void preparation_for_drawing_for_blur(command_list*);
		
		//SSAO���邽�߂�
		void preparation_for_drawing_for_SSAO(command_list*);

		//�R�s�[�֎~
		perapolygon_renderer(const perapolygon_renderer&) = delete;
		perapolygon_renderer& operator=(const perapolygon_renderer&) = delete;
		//���[�u�\
		perapolygon_renderer(perapolygon_renderer&&) noexcept;
		perapolygon_renderer& operator=(perapolygon_renderer&&) noexcept;

	};


}