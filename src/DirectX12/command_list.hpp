#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class pipeline_state;
	template<bool>
	class texture_shader_resource_base;

	class command_list
	{
		ID3D12CommandAllocator* m_allocator = nullptr;
		ID3D12CommandQueue* m_queue = nullptr;

		ID3D12GraphicsCommandList* m_list = nullptr;

		//��
		ID3D12Fence* m_fence = nullptr;
		HANDLE m_fence_event = nullptr;
		UINT64 m_fence_value = 1;

	public:
		command_list() = default;
		~command_list();

		bool initialize(device*);

		ID3D12GraphicsCommandList* get();
		ID3D12CommandQueue* get_queue();
		ID3D12CommandAllocator* get_allocator();

		//�R�}���h�̎��s
		void execute();
		
		//�R�}���h�̃N���A
		//�����͏����ݒ肵�����p�C�v���C���X�e�[�g
		//nullptr�ł��悢
		//pipelinestate�N���X���Ȃ�����
		void clear(pipeline_state* pipelineState = nullptr);

		//�e�N�X�`����src����dst�ɃR�s�[
		void excute_copy_texture(texture_shader_resource_base<true>* src, texture_shader_resource_base<false>* dst);

		//
		//���̑��̃����o�֐��͓K�X�ǉ����Ă���
		//

		//ViewPort�ƃV�U�[��`�͂����ɒu������
		//Draw�̒���
	};

}