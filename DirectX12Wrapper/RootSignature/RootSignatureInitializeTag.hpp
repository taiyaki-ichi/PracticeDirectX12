#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<tuple>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	//
	//���[�g�V�O�l�`�������������鎞�Ɏg�p����^�O
	//

	//�f�B�X�N���v�^�����W�̃^�O
	namespace DescriptorRangeTag 
	{
		//�V�F�[�_���\�[�X�r���[�p
		struct SRV;

		//�A���I�[�_�[�h�A�N�Z�X�r���[�p
		struct UAV;

		//�R���X�^���g�o�b�t�@�r���[�p
		struct CBV;

		//�T���v���[�p
		struct Sampler;
	}

	//DescriptorRangeTag���܂Ƃ߂�p�̃^�O
	template<typename... DescriptorRangeTags>
	struct DescriptorTableTag;

	//DescriptorTableTag���܂Ƃ߂�p�̃^�O
	template<typename... DescriptorTableTags>
	struct DescriptorTableArrayTag;


	//�X�^�e�B�b�N�T���v���[���w�肷��Ƃ��Ɏg�p
	namespace StaticSamplerTag
	{
		//���������̒ʏ�̕`�ʗp
		struct Standard;

		//�g�D�[���p
		struct Toon;

		//�V���h�E�}�b�s���O�p
		struct ShadowMapping;
	}

	//�X�^�e�B�b�N�T���v���[���܂Ƃ߂�p
	template<typename... StaticSamplerTags>
	struct StaticSamplersTag;


	//
	//
	//


	namespace DescriptorRangeTag
	{
		struct SRV {
			constexpr static auto value = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		};

		struct UAV {
			constexpr static auto value = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		};

		struct CBV {
			constexpr static auto value = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		};

		struct Sampler {
			constexpr static auto value = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		};
	}

	template<typename... DescriptorRangeTags>
	struct DescriptorTableTag{
		using Types = std::tuple<DescriptorRangeTags...>;
	};

	template<typename... DescriptorTableTags>
	struct DescriptorTableArrayTag {
		using Types = std::tuple<DescriptorTableTags...>;
	};

	namespace StaticSamplerTag
	{
		struct Standard {};
		struct Toon {};
		struct ShadowMapping {};
	}

	//�X�^�e�B�b�N�T���v���[���܂Ƃ߂�p
	template<typename... StaticSamplerTags>
	struct StaticSamplersTag {
		using Types = std::tuple<StaticSamplerTags...>;
	};
}