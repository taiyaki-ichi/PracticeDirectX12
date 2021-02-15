#include"depth_stencil_buffer.hpp"

namespace DX12
{
	bool depth_stencil_buffer::initialize(device* device, unsigned int width, unsigned int height)
	{
		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2�����̃e�N�X�`���f�[�^�Ƃ���
		depthResDesc.Width = width;
		depthResDesc.Height = height;
		depthResDesc.DepthOrArraySize = 1;//�e�N�X�`���z��ł��Ȃ���3D�e�N�X�`���ł��Ȃ�
		depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;//�[�x�l�������ݗp�t�H�[�}�b�g
		depthResDesc.SampleDesc.Count = 1;//�T���v����1�s�N�Z��������1��
		depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//���̃o�b�t�@�͐[�x�X�e���V���Ƃ��Ďg�p���܂�
		depthResDesc.MipLevels = 1;
		depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthResDesc.Alignment = 0;

		//�f�v�X�p�q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES depthHeapProp{};
		depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULT��������UNKNOWN�ł悵
		depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//���̃N���A�o�����[���d�v�ȈӖ�������
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.f;//�[���P(�ő�l)�ŃN���A
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit�[�x�l�Ƃ��ăN���A
		depthClearValue.Color[0] = 1.f;
		depthClearValue.Color[1] = 1.f;
		depthClearValue.Color[2] = 1.f;
		depthClearValue.Color[3] = 1.f;

		return resource_base::initialize(device,
			&depthHeapProp, 
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
			&depthClearValue
		);

	}

}