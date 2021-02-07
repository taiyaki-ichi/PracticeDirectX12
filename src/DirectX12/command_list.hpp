#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;
	class pipeline_state;
	template<bool>
	class texture_shader_resource_base;

	class command_list
	{
		ID3D12CommandAllocator* m_allocator = nullptr;
		ID3D12CommandQueue* m_queue = nullptr;

		ID3D12GraphicsCommandList* m_list = nullptr;

		//仮
		ID3D12Fence* m_fence = nullptr;
		HANDLE m_fence_event = nullptr;
		UINT64 m_fence_value = 1;

	public:
		command_list() = default;
		~command_list();

		bool initialize(device*);

		ID3D12GraphicsCommandList* get();
		ID3D12CommandQueue* get_queue();
		ID3D12CommandAllocator* get_allocator();

		//コマンドの実行
		void execute();
		
		//コマンドのクリア
		//引数は初期設定したいパイプラインステート
		//nullptrでもよい
		//pipelinestateクラスをなくす鴨
		void clear(pipeline_state* pipelineState = nullptr);

		//テクスチャをsrcからdstにコピー
		void excute_copy_texture(texture_shader_resource_base<true>* src, texture_shader_resource_base<false>* dst);

		//
		//その他のメンバ関数は適宜追加していく
		//

		//ViewPortとシザー矩形はここに置くかも
		//Drawの中か
	};

}