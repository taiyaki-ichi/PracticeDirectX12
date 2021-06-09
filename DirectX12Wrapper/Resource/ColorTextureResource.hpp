#pragma once
#include"ResourceBase.hpp"
#include"../DescriptorHeap/DescripotrHeapViewTag.hpp"
#include<array>

namespace DX12
{
	//�J���[�e�N�X�`���̌�
	class ColorTextureResourceBase : public ResourceBase
	{
	public:
		virtual ~ColorTextureResourceBase() = default;
		void Initialize(Device*, std::uint32_t width, std::uint32_t height);
	};

	//4�~4�̃J���[�e�N�X�`��
	class ColorTextureResource : public ColorTextureResourceBase
	{
	public:
		void Initialize(Device*, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255);
	};

	template<>
	struct ViewTypeTraits<ColorTextureResource>
	{
		using Type = DescriptorHeapViewTag::Float4ShaderResource;
	};

	//�O���C�O���f�[�V����
	class GrayGradationTextureResource : public ColorTextureResourceBase
	{
	public:
		void Initialize(Device*);
	};

	template<>
	struct ViewTypeTraits<GrayGradationTextureResource>
	{
		using Type = DescriptorHeapViewTag::Float4ShaderResource;
	};

	//
	//
	//

	inline void ColorTextureResourceBase::Initialize(Device* device, std::uint32_t width, std::uint32_t height)
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
		resDesc.SampleDesc.Quality = 0;
		resDesc.MipLevels = 1;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

		ResourceBase::Initialize(
			device,
			&texHeapProp,
			D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
			&resDesc,
			ResourceState::PixcelShaderResource,
			nullptr
		);
	}

	inline void ColorTextureResource::Initialize(Device* device, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
	{
		constexpr std::size_t EDGE_LENGTH = 4;

		ColorTextureResourceBase::Initialize(device, EDGE_LENGTH, EDGE_LENGTH);

		std::array<std::uint8_t, 4 * EDGE_LENGTH * EDGE_LENGTH> data{};
		for (std::size_t i = 0; i < EDGE_LENGTH * EDGE_LENGTH; i++) {
			data[4 * i] = r;
			data[4 * i + 1] = g;
			data[4 * i + 2] = b;
			data[4 * i + 3] = a;
		}

		if (FAILED(Get()->WriteToSubresource(0, nullptr, data.data(), 4 * EDGE_LENGTH, data.size())))
			throw "";
	}

	inline void GrayGradationTextureResource::Initialize(Device* device)
	{
		constexpr std::size_t WIDTH = 4;
		constexpr std::size_t HEIGHT = 256;

		ColorTextureResourceBase::Initialize(device, WIDTH, HEIGHT);

		std::array<std::uint8_t, WIDTH* HEIGHT> data{};
		auto it = data.begin();
		unsigned int c = 0xff;
		for (; it != data.end(); it += WIDTH) {
			auto col = (0xff << 24) | RGB(c, c, c);
			std::fill(it, it + WIDTH, col);
			--c;
		}

		if (FAILED(Get()->WriteToSubresource(0, nullptr, data.data(),
			WIDTH * sizeof(std::uint8_t), sizeof(std::uint8_t) * static_cast<UINT>(data.size()))))
			throw "";
	}

}