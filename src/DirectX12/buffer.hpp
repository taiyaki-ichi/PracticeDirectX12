#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<iostream>
#include<DirectXTex.h>
#include"utility.hpp"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")

namespace graphics
{


	//���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�̐������g�p�Asize�̓}�b�v����\��̔z��̑傫��
	ID3D12Resource* create_buffer(ID3D12Device* device,size_t size) {

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resdesc.Width = size;
		resdesc.Height = 1;
		resdesc.DepthOrArraySize = 1;
		resdesc.MipLevels = 1;
		resdesc.Format = DXGI_FORMAT_UNKNOWN;
		resdesc.SampleDesc.Count = 1;
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		ID3D12Resource* vertBuff = nullptr;
		//����
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertBuff)))) {

			return vertBuff;
		}
		//���s
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}


	//�o�b�t�@�ɏ����}�b�v
	template<typename Value,size_t N>
	bool map(ID3D12Resource* buffer, const Value (&a)[N])
	{
		Value* target = nullptr;
		auto result = buffer->Map(0, nullptr, (void**)&target);
		//���s�����Ƃ�
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(a), std::end(a), target);
		buffer->Unmap(0, nullptr);

		return true;
	}

	//���_�o�b�t�@�r���[�̎擾
	D3D12_VERTEX_BUFFER_VIEW get_vertex_buffer_view(ID3D12Resource* buffer, size_t size, size_t stride)
	{
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = buffer->GetGPUVirtualAddress();//�o�b�t�@�̉��z�A�h���X
		view.SizeInBytes = size;//�S�o�C�g��
		view.StrideInBytes = stride;//1���_������̃o�C�g��
		return view;
	}

	//�C���f�b�N�X�o�b�t�@�r���[�̐����Astride�͕����Ȃ������ŌŒ�
	D3D12_INDEX_BUFFER_VIEW get_index_buffer_view(ID3D12Resource* buffer, size_t size)
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = buffer->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R16_UINT;
		view.SizeInBytes = size;
		return view;
	}

	//�摜�A�b�v���[�h�p�̃o�b�t�@�̐���
	ID3D12Resource* create_texture_unload_buffer(ID3D12Device* device, const DirectX::Image* image)
	{

		D3D12_HEAP_PROPERTIES heapprop{};

		//�}�b�v�\�ɂ���
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;

		//�A�b�v���[�h�p�Ȃ̂�UNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//�P��A�_�v�^�[�̂��߂��ꂼ��0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;


		D3D12_RESOURCE_DESC resdesc{};

		//�P�Ȃ�f�[�^
		resdesc.Format = DXGI_FORMAT_UNKNOWN;
		//�P�Ȃ�o�b�t�@
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

		//�f�[�^�T�C�Y
		resdesc.Width = static_cast<UINT64>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) * image->height;
		resdesc.Height = 1;
		resdesc.DepthOrArraySize = 1;
		resdesc.MipLevels = 1;

		//�A�������f�[�^
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		//���ɂȂ�
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		//���ʂ̃e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ�
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;

		ID3D12Resource* uploadBuff = nullptr;
		//����
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuff)))) {

			return uploadBuff;
		}
		//���s
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}


	//�e�N�X�`���p�̃o�b�t�@�A�R�s�[����鑤
	ID3D12Resource* create_texture_buffer(ID3D12Device* device, const DirectX::TexMetadata& metaData)
	{
		D3D12_HEAP_PROPERTIES heapprop{};

		//�e�N�X�`���p
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;

		//��������UNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		//�P��A�_�v�^�[�̂��߂��ꂼ��0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		//�قƂ��metaData����
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Format = metaData.format;
		resdesc.Width = static_cast<UINT>(metaData.width);//��
		resdesc.Height = static_cast<UINT>(metaData.height);//����
		resdesc.DepthOrArraySize = static_cast<UINT16>(metaData.arraySize);//2D�Ŕz��ł��Ȃ��̂łP
		resdesc.MipLevels = static_cast<UINT16>(metaData.mipLevels);//�~�b�v�}�b�v���Ȃ��̂Ń~�b�v���͂P��
		resdesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData.dimension);//2D�e�N�X�`���p
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		//���ɂȂ�
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		//���ʂ̃e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ�
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;

		ID3D12Resource* textureBuff = nullptr;
		//����
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_COPY_DEST,//�R�s�[��Ȃ̂�
			nullptr,
			IID_PPV_ARGS(&textureBuff)))) {

			return textureBuff;
		}
		//���s
		else {
			std::cout << __func__ << " is failed\n";
			return nullptr;
		}
	}


	//�摜��map�̒��Ŏg�p�A��
	//GetCopyableFootprints�łł���Ȃ炻�ꂪ����
	void copy_image(uint8_t* mapForImage, const DirectX::Image* image) {
		uint8_t* srcAddress = image->pixels;
		size_t rowPitch = alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

		for (int y = 0; y < image->height; ++y) {
			std::copy_n(srcAddress,
				rowPitch,
				mapForImage);//�R�s�[
			//1�s���Ƃ̒�������킹�Ă��
			srcAddress += image->rowPitch;
			mapForImage += rowPitch;
		}
	}

	//�摜�p�A�Ƃ肠�����T���v���ǂ����
	bool map(ID3D12Resource* buffer, const DirectX::Image* image)
	{
		uint8_t* mapforImage = nullptr;
		if (FAILED(buffer->Map(0, nullptr, (void**)&mapforImage))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		copy_image(mapforImage, image);

		buffer->Unmap(0, nullptr);
	}

	//�s��p
	bool map(ID3D12Resource* buffer, const DirectX::XMMATRIX& m)
	{
		DirectX::XMMATRIX* ptr = nullptr;
		if (FAILED(buffer->Map(0, nullptr, (void**)&ptr))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		*ptr = m;
		buffer->Unmap(0, nullptr);
	}
}