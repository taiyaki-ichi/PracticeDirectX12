#pragma once
#include"device.hpp"
#include"texture_shader_resource.hpp"
#include<type_traits>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//仮
	constexpr unsigned int DESCRIPTOR_HEAP_SIZE = 128;

	//ConstantBuffer用とShaderResouce用
	//sapmerはstaticにRootSignatureで設定
	class descriptor_heap
	{
		ID3D12DescriptorHeap* m_descriptor_heap = nullptr;

		unsigned int m_size = 0;
		unsigned int m_offset = 0;
		unsigned int m_stride = 0;
		unsigned int m_increment_size = 0;

	public:
		descriptor_heap() = default;
		~descriptor_heap();

		bool initialize(device* d, unsigned int size);
		//bool initialize(device* d, unsigned int num, unsigned int stride);

		//ビューの作製
		//ディスクリプタヒープをセットするようにgpu_handleを取得できるようにしておく
		D3D12_GPU_DESCRIPTOR_HANDLE create_view(device* device, constant_buffer_resource* resourcePtr);
		D3D12_GPU_DESCRIPTOR_HANDLE create_view(device* device, texture_shader_resource* resourcePtr);

		//offsetを0にする
		void reset() noexcept;

		ID3D12DescriptorHeap*& get_ptr() noexcept;

		//先頭からstride×num分進んだハンドルの位置
		//0スタート
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(unsigned int num = 0);
	};

}