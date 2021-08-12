#pragma once

namespace DX12
{
	//DescriptorHeapにViewを作る時に使用
	namespace ViewTypeTag {

		//定数バッファ
		struct ConstantBuffer {};

		//深度ステンシル
		struct DepthStencilBuffer {};

		//SRV用
		struct ShaderResource {};

		//キューブマップ用
		struct CubeMap {};

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