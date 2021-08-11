#pragma once

namespace DX12
{
	//DescriptorHeapにViewを作る時に使用
	namespace DescriptorHeapViewTag {

		//定数バッファ
		struct ConstantBuffer {};

		//深度ステンシル
		struct DepthStencilBuffer {};

		//RGBAのテクスチャのようなfloatが4つからなるデータ
		struct Float4ShaderResource {};

		//Rのみのテクスチャのようなfloatが1つからなるデータ
		struct FloatShaderResource {};

		//キューブマップ用
		struct CubeMapResource {};

		//キューブマップ用のデプスステンシル
		struct CubeMapDepthStencilBuffer {};

		//アンオーダ用のタグ
		struct UnorderedAccessResource {};
	}

	//namespace DescriptorHeapViewTag の型をViewTypeとしてusingする
	template<typename T>
	struct DefaultViewTypeTraits
	{
		using Type;
	};
}