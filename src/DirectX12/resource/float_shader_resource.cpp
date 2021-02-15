#include"float_shader_resource.hpp"

namespace DX12
{

	bool float_shader_resource::initialize(device* device, unsigned int width, unsigned int height, std::optional<D3D12_CLEAR_VALUE> clearValue)
	{
		D3D12_RESOURCE_DESC resdesc{};
		resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2�����̃e�N�X�`���f�[�^�Ƃ���
		resdesc.Width = width;//���ƍ����̓����_�[�^�[�Q�b�g�Ɠ���
		resdesc.Height = height;//��ɓ���
		resdesc.DepthOrArraySize = 1;//�e�N�X�`���z��ł��Ȃ���3D�e�N�X�`���ł��Ȃ�
		resdesc.Format = DXGI_FORMAT_R32_FLOAT;
		resdesc.SampleDesc.Count = 1;//�T���v����1�s�N�Z��������1��
		resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		resdesc.MipLevels = 1;
		resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resdesc.Alignment = 0;

		D3D12_HEAP_PROPERTIES heapprop{};
		heapprop.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULT��������UNKNOWN�ł悵
		heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_CLEAR_VALUE* clear = nullptr;
		if (clearValue)
			clear = &clearValue.value();

		return resource_base::initialize(device,
			&heapprop,
			D3D12_HEAP_FLAG_NONE,
			&resdesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //�f�v�X�������݂Ɏg�p
			clear
		);
	}
}