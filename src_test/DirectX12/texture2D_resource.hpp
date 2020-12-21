#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//�e�N�X�`���̃��\�[�X
	//�e�N�X�`����p�̂ŃX�N���v�^�q�[�v��View�����
	//�����A�ǂ�����ăR�s�[���č쐬���悤���ȁB�B�B
	class texture2D_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		texture2D_resource() = default;
		~texture2D_resource();

		//device���ŌĂ΂��
		//�e�N�X�`���̃R�s�[�������ł���Ă��܂������B�B�B
		bool initialize(device*,char* fileName);

		//�f�B�X�N���v�^�q�[�v�Ɋ֘A�t����Ƃ��Ɏg�p
		ID3D12Resource* get();
	};
}
