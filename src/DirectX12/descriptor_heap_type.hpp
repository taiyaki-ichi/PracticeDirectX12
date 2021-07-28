#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

//�f�B�X�N���v�^�q�[�v�̌^�֘A

namespace DX12
{

	//view�����p
	//�������@�̐ÓI�Ȋ֐�����`����Ă���
	//���ƁA�L���ȃf�B�X�N���v�^�q�[�v�̃^�C�v���ǂ����̔����
	namespace view_type {

		//�萔�o�b�t�@
		struct constant_buffer {};

		//�[�x�X�e���V��
		struct depth_stencil_buffer {};

		//RGBA�̃e�N�X�`���̂悤�ȃf�[�^
		struct float4_shader_resource {};

		//R�݂̂̃e�N�X�`���̂悤�ȃf�[�^
		struct float_shader_resource {};
	}


	//�쐻����View�̃^�C�v���w�肷��g���C�c
	//namespace view_type�̌^��using����
	//descriptor_heap::create_view���Ŏg�p
	template<typename T>
	struct DefaultViewTypeTraits {
		using view_type;
	};


	//���ۂ̃��\�[�X�̃|�C���^�����o�����߂̃|���V�[
	//descriptor_heap::create_view�Ȃ��Ŏg�p
	template<typename T>
	struct GetResourcePtrPolicy {
		static ID3D12Resource* get_resource_ptr(T*) {
			return nullptr;
		}
	};


	//�^�C�v�w��p
	//�������p�̊֐��̒�`
	//�C���N�������g�̕��̎擾�p�֐��Ƃ�
	//��`��descriptor_heap.hpp����
	namespace descriptor_heap_type {
		struct CBV_SRV_UAV;
		struct DSV;
		struct RTV;
	};


	
}