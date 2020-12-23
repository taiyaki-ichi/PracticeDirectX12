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
	class texture_shader_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		texture_shader_resource() = default;
		~texture_shader_resource();

		//device���ŌĂ΂��
		//�e�N�X�`���̃R�s�[�������ł���Ă��܂������B�B�B
		bool initialize(device*,char* fileName);

		//�f�B�X�N���v�^�q�[�v�Ɋ֘A�t����Ƃ��Ɏg�p
		ID3D12Resource* get() const noexcept;
	};
}
