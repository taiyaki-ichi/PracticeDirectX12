#pragma once

namespace DX12
{

	//view生成用
	//生成方法の静的な関数が定義されている
	//あと、有効なディスクリプタヒープのタイプかどうかの判定も
	namespace resource_type {
		struct CBV {};
		struct SRV {};
		struct DSV {};
		struct RTV {};

		//仮
		struct R32 {};
	};
}