#include"texture_resource.hpp"
#include"upload_resource.hpp"
#include"../utility.hpp"
#include"../device.hpp"
#include"../command_list.hpp"

namespace DX12
{
	bool texture_resource::initialize(device* device, command_list* cl, const DirectX::TexMetadata* metaData, const DirectX::ScratchImage* scratchImage)
	{
		//�摜�̃|�C���^
		auto image = scratchImage->GetImage(0, 0, 0);

		//���Ԃ̃o�b�t�@
		upload_resource src{};
		src.initialize(device, static_cast<UINT64>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) * image->height);

		//�f�[�^���}�b�v
		src.map(*image);

	
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resdesc.SampleDesc.Count = 1;
		resdesc.SampleDesc.Quality = 0;
		resdesc.Format = metaData->format;
		resdesc.Width = static_cast<UINT>(metaData->width);//��
		resdesc.Height = static_cast<UINT>(metaData->height);//����
		resdesc.DepthOrArraySize = static_cast<UINT16>(metaData->arraySize);//2D�Ŕz��ł��Ȃ��̂łP
		resdesc.MipLevels = static_cast<UINT16>(metaData->mipLevels);//�~�b�v�}�b�v���Ȃ��̂Ń~�b�v���͂P��
		resdesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData->dimension);//2D�e�N�X�`���p
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;
		//��������UNKNOWN
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		//�P��A�_�v�^�[�̂��߂��ꂼ��0
		heapprop.CreationNodeMask = 0;
		heapprop.VisibleNodeMask = 0;

		ID3D12Resource* dstPtr = nullptr;

		if (FAILED(device->get()->CreateCommittedResource(
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&dstPtr)))) {
			return false;
		}

		//
		//�R�s�[�̂��߂̍\���̂̐ݒ�
		//

		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		D3D12_TEXTURE_COPY_LOCATION dstLocation{};

		srcLocation.pResource = src.get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			auto desc = dstPtr->GetDesc();
			device->get()->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(metaData->width);
		srcLocation.PlacedFootprint.Footprint.Height = static_cast<UINT>(metaData->height);
		srcLocation.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metaData->depth);
		srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignment_size(image->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
		srcLocation.PlacedFootprint.Footprint.Format = image->format;

		dstLocation.pResource = dstPtr;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;

		//
		//�߂�l�̃��\�[�X�ɃR�s�[����R�}���h�����s
		//
		cl->get()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
		cl->get()->Close();
		cl->execute();
		cl->clear();


		return resource_base::initialize(dstPtr);
	}
}