#pragma once

namespace ichi
{
	//descriptor��view�����Ƃ��̎��ʎq�Ƃ��Ďg�p
	//�r���[����肽���N���X��using�����Ďg��
	//�Ƃ肠���������ɒu���Ă���
	
	//�V�F�[�_���\�[�X�p
	struct shader_resource_tag {};

	//�萔�o�b�t�@�p
	struct constant_buffer_tag {};

	//�[�x�o�b�t�@���e�N�X�`���Ƃ��Ĉ������߂�
	struct depth_buffer_tag {};


	//view�����p
	//�������@�̐ÓI�Ȋ֐�����`����Ă���
	//���ƁA�L���ȃf�B�X�N���v�^�q�[�v�̃^�C�v���ǂ����̔����
	namespace create_view_type {
		struct CBV {};
		struct SRV {};
		struct DSV {};
	};
}