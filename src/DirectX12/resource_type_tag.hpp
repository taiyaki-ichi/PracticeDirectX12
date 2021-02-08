#pragma once

namespace ichi
{

	//view生成用
	//生成方法の静的な関数が定義されている
	//あと、有効なディスクリプタヒープのタイプかどうかの判定も
	namespace create_view_type {
		struct CBV {};
		struct SRV {};
		struct DSV {};
	};
}