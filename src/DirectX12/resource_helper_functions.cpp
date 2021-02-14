#include"resource_helper_functions.hpp"
#include"utility.hpp"
#include"device.hpp"
#include"command_list.hpp"

namespace DX12
{

	resource create_constant_resource(device* device, unsigned int size)
	{
		resource result{};

		//�T�C�Y��16�̔{�����Ⴀ�Ȃ��Ƃ����Ȃ��̂ŃA���C�����g
		size = (size + 0xff) & ~0xff;

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

		if (!result.initialize(device,
			&heapprop, 
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, 
			nullptr))
		{
			std::cout << __func__ << " is failed\n";
		}

		return result;
	}


	resource create_texture_resource(device* device, command_list* cl, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage)
	{
		//���ԃo�b�t�@�p
		resource src{};
		//���ۂ̃f�[�^
		resource dst{};

		//�摜�̃|�C���^
		auto image = scratchImage->GetImage(0, 0, 0);

		D3D12_HEAP_PROPERTIES heapprop{};
		D3D12_RESOURCE_DESC resdesc{};
		//�v���p�e�B�[�̋��ʕ���
		//���ɂȂ�
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		//���ʂ̃e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ�
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;


		//
		//�܂��͒��ԃo�b�t�@���쐻
		//
		//�}�b�v�\�ɂ���
		heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
		//�A�b�v���[�h�p�Ȃ̂�UNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		//�P��A�_�v�^�[�̂��߂��ꂼ��0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;
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

		//�A�b�v���[�h����̂Ń��\�[�X�̏�Ԃ�READ
		src.initialize(device, &heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);


		//
		//���ɖ߂�l�ƂȂ郊�\�[�X�̏�����
		//
		//�e�N�X�`���p
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		//��������UNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		//�P��A�_�v�^�[�̂��߂��ꂼ��0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;
		resdesc.Format = metaData->format;
		resdesc.Width = static_cast<UINT>(metaData->width);//��
		resdesc.Height = static_cast<UINT>(metaData->height);//����
		resdesc.DepthOrArraySize = static_cast<UINT16>(metaData->arraySize);//2D�Ŕz��ł��Ȃ��̂łP
		resdesc.MipLevels = static_cast<UINT16>(metaData->mipLevels);//�~�b�v�}�b�v���Ȃ��̂Ń~�b�v���͂P��
		resdesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData->dimension);//2D�e�N�X�`���p
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		//������Ԃ�DEST
		dst.initialize(device, &heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);


		//
		//�e�N�X�`���̃f�[�^���}�b�v
		//
		map_to_resource(&src, *image);


		//
		//�e�N�X�`�����R�s�[���邽�߂��ꂼ��̃��P�[�V�����̍\���̂̐ݒ�
		//
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		D3D12_TEXTURE_COPY_LOCATION dstLocation{};
		
		srcLocation.pResource = src.get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			auto desc = dst.get()->GetDesc();
			device->get()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData->width);
		srcLocation.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData->height);
		srcLocation.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData->depth);
		srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		srcLocation.PlacedFootprint.Footprint.Format = image->format;

		dstLocation.pResource = dst.get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;


		//
		//�߂�l�̃��\�[�X�ɃR�s�[����R�}���h�����s
		//
		cl->get()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
		cl->get()->Close();
		cl->execute();
		cl->clear();

		return dst;
	}
	

	resource create_simple_resource(device* device, unsigned int width, unsigned int height,
		DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flag, D3D12_RESOURCE_STATES state, D3D12_CLEAR_VALUE clearValue)
	{
		resource result{};

		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2�����̃e�N�X�`���f�[�^�Ƃ���
		resdesc.Width = width;//���ƍ����̓����_�[�^�[�Q�b�g�Ɠ���
		resdesc.Height = height;//��ɓ���
		resdesc.DepthOrArraySize = 1;//�e�N�X�`���z��ł��Ȃ���3D�e�N�X�`���ł��Ȃ�
		resdesc.Format = format;
		resdesc.SampleDesc.Count = 1;//�T���v����1�s�N�Z��������1��
		resdesc.Flags = flag;
		resdesc.MipLevels = 1;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resdesc.Alignment = 0;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULT��������UNKNOWN�ł悵
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;


		if (!result.initialize(device,
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			state,
			&clearValue))
		{
			std::cout << __func__ << " is failed\n";
		}

		return result;
	}

	resource create_depth_resource(device* device, unsigned int width, unsigned int height)
	{
		resource result{};

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

		if (!result.initialize(
			device,
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
			&depthClearValue
		)) {
			std::cout << "create depth initialize is failed\n";
		}

		return result;
	}


	bool map_to_resource(resource* resource, const DirectX::Image& image)
	{
		uint8_t* mapforImage = nullptr;
		if (FAILED(resource->get()->Map(0, nullptr, (void**)&mapforImage))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		uint8_t* srcAddress = image.pixels;
		size_t rowPitch = alignment_size(image.rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

		for (size_t y = 0; y < image.height; ++y) {
			std::copy_n(srcAddress,
				rowPitch,
				mapforImage);//�R�s�[
			//1�s���Ƃ̒�������킹�Ă��
			srcAddress += image.rowPitch;
			mapforImage += rowPitch;
		}

		resource->get()->Unmap(0, nullptr);

		return true;
	}

}