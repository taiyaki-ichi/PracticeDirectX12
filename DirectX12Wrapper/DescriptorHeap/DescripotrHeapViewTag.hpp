#pragma once

namespace DX12
{
	//DescriptorHeap��View����鎞�Ɏg�p
	namespace DescriptorHeapViewTag {

		//�萔�o�b�t�@
		struct ConstantBuffer {};

		//�[�x�X�e���V��
		struct DepthStencilBuffer {};

		//
		struct ShaderResource {};

		//�L���[�u�}�b�v�p
		struct CubeMap {};

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