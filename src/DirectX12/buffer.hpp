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


}