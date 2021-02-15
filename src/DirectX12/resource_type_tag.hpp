#pragma once

namespace DX12
{

	//view生成用
	//生成方法の静的な関数が定義されている
	//あと、有効なディスクリプタヒープのタイプかどうかの判定も
	namespace view_type {

		//定数バッファ
		struct constant_buffer {};

		//深度ステンシル
		struct depth_stencil_buffer {};

		//RGBAのテクスチャのようなデータ
		struct float4_shader_resource {};

		//Rのみのテクスチャのようなデータ
		struct float_shader_resource {};
	}
	
}