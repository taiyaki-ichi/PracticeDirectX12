#pragma once

namespace DX12
{
	//DescriptorHeap��View����鎞�Ɏg�p
	namespace DescriptorHeapViewTag {

		//�萔�o�b�t�@
		struct ConstantBuffer {};

		//�[�x�X�e���V��
		struct DepthStencilBuffer {};

		//RGBA�̃e�N�X�`���̂悤��float��4����Ȃ�f�[�^
		struct Float4ShaderResource {};

		//R�݂̂̃e�N�X�`���̂悤��float��1����Ȃ�f�[�^
		struct FloatShaderResource {};

		//�L���[�u�}�b�v�p
		struct CubeMapResource {};

		//�L���[�u�}�b�v�p�̃f�v�X�X�e���V��
		struct CubeMapDepthStencilBuffer {};

		//�A���I�[�_�p�̃^�O
		struct UnorderedAccessResource {};
	}

	//namespace DescriptorHeapViewTag �̌^��ViewType�Ƃ���using����
	template<typename T>
	struct DefaultViewTypeTraits
	{
		using Type;
	};
}