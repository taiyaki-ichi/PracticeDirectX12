#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//�萔�o�b�t�@
	//��p�̃f�B�X�N���v�^�q�[�v��View�����
	class constant_buffer_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		constant_buffer_resource() = default;
		~constant_buffer_resource();

		//device���Ŏg�p
		//size�̓A���C�����g���ꂽ�l
		bool initialize(device*,unsigned int size);

		template<typename T>
		bool map(T&&) {

		}

		//�f�B�X�N���v�^�q�[�v�Ɋ֘A�Â���Ƃ��Ɏg�p����
		ID3D12Resource* get();


	};
}