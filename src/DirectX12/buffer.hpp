#pragma once
#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<DirectXTex.h>
#include<iostream>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"DirectXTex.lib")

namespace graphics
{
	size_t alignment_size(size_t size, size_t alignment) {
		return size + alignment - size % alignment;
	}

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
	ID3D12Resource* create_texture_unload_buffer(ID3D12Device* device,const DirectX::Image* image)
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

	namespace {


		//�摜��map�̒��Ŏg�p�A��
		//GetCopyableFootprints�łł���Ȃ炻�ꂪ����
		void copy_image(uint8_t* mapForImage,const DirectX::Image* image) {
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


	//�R�s�[�Ɏg�p����\���̂�2�Ԃ�
	//�A�b�v���[�h���ꂽ���A�R�s�[�������A�A�̏�
	std::pair<D3D12_TEXTURE_COPY_LOCATION, D3D12_TEXTURE_COPY_LOCATION> 
		get_texture_copy_location(ID3D12Device* device, ID3D12Resource* uploadBuff, ID3D12Resource* textureBuff, const DirectX::TexMetadata& metaData, const DirectX::Image* image)
	{
		D3D12_TEXTURE_COPY_LOCATION src{};
		D3D12_TEXTURE_COPY_LOCATION dst{};

		dst.pResource = textureBuff;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		src.pResource = uploadBuff;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
		UINT nrow;
		UINT64 rowsize, size;
		auto desc = textureBuff->GetDesc();
		device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
		src.PlacedFootprint = footprint;
		src.PlacedFootprint.Offset = 0;
		src.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData.width);
		src.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData.height);
		src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData.depth);
		src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		src.PlacedFootprint.Footprint.Format = image->format;

		return { src,dst };
	}


	void copy_texture(ID3D12CommandAllocator* allocator,ID3D12GraphicsCommandList* commandList, ID3D12CommandQueue* queue,
		ID3D12Fence* fence, UINT64& fenceVal,
		D3D12_TEXTURE_COPY_LOCATION& uploadLocation, D3D12_TEXTURE_COPY_LOCATION& textureLocation)
	{

		//allocator->Reset();
		//commandList->Reset(allocator, nullptr);

		commandList->CopyTextureRegion(&textureLocation, 0, 0, 0, &uploadLocation, nullptr);

		D3D12_RESOURCE_BARRIER BarrierDesc{};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = textureLocation.pResource;
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		commandList->ResourceBarrier(1, &BarrierDesc);
		commandList->Close();

		//���s
		ID3D12CommandList* cmdlists[] = { commandList };
		queue->ExecuteCommandLists(1, cmdlists);
		//�҂�
		
		queue->Signal(fence, ++fenceVal);
		//std::cout << UINT64_MAX << "\n";
		
		if (fence->GetCompletedValue() != fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		
		allocator->Reset();//�L���[���N���A
		commandList->Reset(allocator, nullptr);
	}

}