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
	class texture2D_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		texture2D_resource() = default;
		~texture2D_resource();

		//device内で呼ばれる
		//テクスチャのコピーもここでやってしまおうか。。。
		bool initialize(device*,char* fileName);

		//ディスクリプタヒープに関連付けるときに使用
		ID3D12Resource* get();
	};
}
