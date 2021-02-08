#include"mmd_depth_buffer.hpp"
#include"DirectX12/device.hpp"

namespace ichi
{
	std::optional<std::pair<ID3D12Resource*, ID3D12Resource*>>
		create_mmd_depth_buffers(device* device, unsigned int windowWidth, unsigned int windowHeight, unsigned int shadowDifinition)
	{

		ID3D12Resource* depthBuffer = nullptr;

		//�[�x�o�b�t�@�̎d�l
		D3D12_RESOURCE_DESC depthResDesc{};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2�����̃e�N�X�`���f�[�^�Ƃ���
		depthResDesc.Width = windowWidth;//���ƍ����̓����_�[�^�[�Q�b�g�Ɠ���
		depthResDesc.Height = windowHeight;//��ɓ���
		depthResDesc.DepthOrArraySize = 1;//�e�N�X�`���z��ł��Ȃ���3D�e�N�X�`���ł��Ȃ�
		depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;//�[�x�l�������ݗp�t�H�[�}�b�g
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
		depthClearValue.DepthStencil.Depth = 1.0f;//�[���P(�ő�l)�ŃN���A
		depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit�[�x�l�Ƃ��ăN���A

		if (FAILED(device->get()->CreateCommittedResource(
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
			&depthClearValue,
			IID_PPV_ARGS(&depthBuffer)
		))) {
			std::cout << "mmd depth_buffer initialize is failed\n";
			return std::nullopt;
		}


		ID3D12Resource* lightDepthBuffer = nullptr;

		depthResDesc.Width = shadowDifinition;
		depthResDesc.Height = shadowDifinition;
		depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;

		if (FAILED(device->get()->CreateCommittedResource(
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
			&depthClearValue,
			IID_PPV_ARGS(&lightDepthBuffer)
		))) {
			std::cout << "mmd light depth initialize is failed\n";
			if (depthBuffer)
				depthBuffer->Release();
			return std::nullopt;
		}


		return std::make_pair(depthBuffer, lightDepthBuffer);
	}


}