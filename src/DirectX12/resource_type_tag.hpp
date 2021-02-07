#pragma once

namespace ichi
{
	//descriptorにviewを作るときの識別子として使用
	//ビューを作りたいクラスにusingさせて使う
	//とりあえずここに置いておく
	
	//シェーダリソース用
	struct shader_resource_tag {};

	//定数バッファ用
	struct constant_buffer_tag {};

	//深度バッファをテクスチャとして扱うために
	struct depth_buffer_tag {};
}