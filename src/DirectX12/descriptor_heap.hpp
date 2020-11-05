#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<iostream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace graphics
{
	//�f�B�X�N���v�^�q�[�v�̐���
	ID3D12DescriptorHeap* create_texture_descriptor_heap(ID3D12Device* device)
	{
		ID3D12DescriptorHeap* descHeap = nullptr;
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
		descHeapDesc.NodeMask = 0;//�}�X�N��0
		descHeapDesc.NumDescriptors = 1;//�r���[�͍��̂Ƃ���P����
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�V�F�[�_���\�[�X�r���[(����ђ萔�AUAV��)

		if (FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap)))) {
			std::cout << __func__ << " is falied\n";
			return nullptr;
		}
		return descHeap;
	}

	//�e�N�X�`���p�̐ݒ�
	void set_texture_view(ID3D12Device* device, ID3D12DescriptorHeap* dscHeap, ID3D12Resource* textureBuffer)
	{
		auto desc = textureBuffer->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format;//DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA(0.0f�`1.0f�ɐ��K��)
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//��q
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
		srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

		device->CreateShaderResourceView(textureBuffer, //�r���[�Ɗ֘A�t����o�b�t�@
			&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
			dscHeap->GetCPUDescriptorHandleForHeapStart()//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
		);
	}



}