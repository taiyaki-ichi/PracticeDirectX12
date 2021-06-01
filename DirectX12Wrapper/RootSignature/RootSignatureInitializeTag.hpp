#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<tuple>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	//
	//ルートシグネチャを初期化する時に使用するタグ
	//

	//ディスクリプタレンジのタグ
	namespace DescriptorRangeTag 
	{
		//シェーダリソースビュー用
		struct SRV;

		//アンオーダードアクセスビュー用
		struct UAV;

		//コンスタントバッファビュー用
		struct CBV;

		//サンプラー用
		struct Sampler;
	}

	//DescriptorRangeTagをまとめる用のタグ
	template<typename... DescriptorRangeTags>
	struct DescriptorTableTag;

	//DescriptorTableTagをまとめる用のタグ
	template<typename... DescriptorTableTags>
	struct DescriptorTableArrayTag;


	//スタティックサンプラーを指定するときに使用
	namespace StaticSamplerTag
	{
		//おおかたの通常の描写用
		struct Standard;

		//トゥーン用
		struct Toon;

		//シャドウマッピング用
		struct ShadowMapping;
	}

	//スタティックサンプラーをまとめる用
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

	//スタティックサンプラーをまとめる用
	template<typename... StaticSamplerTags>
	struct StaticSamplersTag {
		using Types = std::tuple<StaticSamplerTags...>;
	};
}