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
	}

	//namespace DescriptorHeapViewTag の型をViewTypeとしてusingする
	template<typename T>
	struct ViewTypeTraits
	{
		using Type;
	};

	template<typename T>
	struct ResourcePtrTraits
	{
		//デフォルト実装
		static ID3D12Resource* Get(T* t) {
			return t->Get();
		}
	};


}