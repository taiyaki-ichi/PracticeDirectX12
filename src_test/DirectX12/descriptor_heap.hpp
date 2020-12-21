#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;


	//��
	constexpr unsigned int DESCRIPTOR_HEAP_SIZE = 128;

	//ConstantBuffer�p��ShaderResouce�p��2���
	//sapmer��static��RootSignature�Őݒ�
	template<typename Value>
	class descriptor_heap
	{
		ID3D12Resource* m_resource = nullptr;

		unsigned int m_offset = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap();

		bool initialize(unsigned int size);

		//�R���e�i�������Ƀf�B�X�N���v�^�q�[�v���Ƀr���[���쐻
		//�R���e�i�̃T�C�Y��DESCRIPTOR_HEAP_SIZE�ȉ�
		//�`�ʖ��ɌĂяo���ăr���[���쐻
		//�e�N�X�`�����萔�o�b�t�@���ŏ������ς��
		//����󂯓����悤�ɂ��邩
		void create_view(Value&&);

		//offset��0�ɂ���
		void reset();
	};

}