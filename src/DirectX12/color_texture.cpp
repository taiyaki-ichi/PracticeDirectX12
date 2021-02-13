#include"color_texture.hpp"
#include"device.hpp"
#include<vector>

namespace DX12
{
	
	bool color_texture_resource_base::initialize(device* device, unsigned int width, unsigned int height)
	{
		D3D12_HEAP_PROPERTIES texHeapProp{};
		texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
		texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
		texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
		texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
		texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

		D3D12_RESOURCE_DESC resDesc{};
		resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		resDesc.Width = width;//��
		resDesc.Height = height;//����
		resDesc.DepthOrArraySize = 1;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;//
		resDesc.MipLevels = 1;//
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

		if (!resource::initialize(
			device,
			&texHeapProp,
			D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr
			)) {
			std::cout << "color_texture_resource_base::initialize is failed\n";
			return false;
		}

		return true;
	}

	bool gray_gradation_texture_resource::initialize(device* device)
	{
		if (!color_texture_resource_base::initialize(device, 4, 256))
			return false;

		//�オ�����ĉ��������e�N�X�`���f�[�^���쐬
		std::vector<unsigned int> data(4 * 256);
		auto it = data.begin();
		unsigned int c = 0xff;
		for (; it != data.end(); it += 4) {
			auto col = (0xff << 24) | RGB(c, c, c);
			std::fill(it, it + 4, col);
			--c;
		}

		if (FAILED(get()->WriteToSubresource(0, nullptr, data.data(),
			4 * sizeof(unsigned int), sizeof(unsigned int) * static_cast<UINT>(data.size()))))
			return false;

		return true;
	}
	
}