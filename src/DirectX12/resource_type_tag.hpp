#pragma once

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
	
}