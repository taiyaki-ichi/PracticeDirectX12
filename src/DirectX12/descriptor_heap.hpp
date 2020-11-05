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
	ID3D12DescriptorHeap* create_basic_descriptor_heap(ID3D12Device* device,unsigned int viewNum)
	{
		ID3D12DescriptorHeap* descHeap = nullptr;
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
		descHeapDesc.NodeMask = 0;//�}�X�N��0
		descHeapDesc.NumDescriptors = viewNum;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�V�F�[�_���\�[�X�r���[(����ђ萔�AUAV��)

		if (FAILED(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap)))) {
			std::cout << __func__ << " is falied\n";
			return nullptr;
		}
		return descHeap;
	}

	//�e�N�X�`���P�ƒ萔�P��
	void set_basic_view(ID3D12Device* device, ID3D12DescriptorHeap* dscHeap, ID3D12Resource* textureBuffer, ID3D12Resource* constBuffer)
	{
		//�f�X�N���v�^�̐擪�n���h�����擾���Ă���
		auto basicHeapHandle = dscHeap->GetCPUDescriptorHandleForHeapStart();

		auto desc = textureBuffer->GetDesc();

		//�e�N�X�`��
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = desc.Format;//DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA(0.0f�`1.0f�ɐ��K��)
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//��q
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
		srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

		device->CreateShaderResourceView(textureBuffer, //�r���[�Ɗ֘A�t����o�b�t�@
			&srvDesc, //��قǐݒ肵���e�N�X�`���ݒ���
			dscHeap->GetCPUDescriptorHandleForHeapStart()//�q�[�v�̂ǂ��Ɋ��蓖�Ă邩
		);

		basicHeapHandle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		//�萔
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
		cbvDesc.BufferLocation = constBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(constBuffer->GetDesc().Width);
		//�萔�o�b�t�@�r���[�̍쐬
		device->CreateConstantBufferView(&cbvDesc, basicHeapHandle);
	}

	//�e�N�X�`���̂ŃX�N���v�^�����W
	D3D12_DESCRIPTOR_RANGE get_texture_descriptor_range(unsigned int slotNum=0)
	{
		D3D12_DESCRIPTOR_RANGE desc{};
		//�e�N�X�`���̐�
		desc.NumDescriptors = 1;
		desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		//�[���ԃX���b�g����
		desc.BaseShaderRegister = slotNum;
		desc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		return desc;
	}

	//�萔�o�b�t�@�̂ł�������Ղ������W
	D3D12_DESCRIPTOR_RANGE get_constant_descriptor_range(unsigned int slotNum)
	{
		D3D12_DESCRIPTOR_RANGE desc{};
		desc.NumDescriptors = 1;
		desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		desc.BaseShaderRegister = slotNum;
		desc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		return desc;
	}


}