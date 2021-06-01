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
	}

	//namespace DescriptorHeapViewTag �̌^��ViewType�Ƃ���using����
	template<typename T>
	struct ViewTypeTraits
	{
		using Type;
	};

	template<typename T>
	struct ResourcePtrTraits
	{
		//�f�t�H���g����
		static ID3D12Resource* Get(T* t) {
			return t->Get();
		}
	};


}