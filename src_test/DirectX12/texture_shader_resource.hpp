#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//テクスチャのリソース
	//テクスチャ専用のでスクリプタヒープにViewを作る
	//ただ、どうやってコピーして作成しようかな。。。
	class texture_shader_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		texture_shader_resource() = default;
		~texture_shader_resource();

		//device内で呼ばれる
		//テクスチャのコピーもここでやってしまおうか。。。
		bool initialize(device*,char* fileName);

		//ディスクリプタヒープに関連付けるときに使用
		ID3D12Resource* get() const noexcept;
	};
}
