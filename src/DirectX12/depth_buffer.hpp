#pragma once
#include"resource_helper_functions.hpp"
#include"command_list.hpp"
#include"resource.hpp"
#include"device.hpp"
#include"descriptor_heap.hpp"
#include<array>
#include<vector>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class device;
	template<typename>
	class descriptor_heap;
	namespace descriptor_heap_type {
		struct DSV;
	};

	class resource;
	class command_list;
	
	//共通で使いまわすような深度バッファたちをまとめたいクラス
	template<unsigned int Size>
	class depth_buffer
	{
		//深度用のディスクリプタヒープ
		descriptor_heap<descriptor_heap_type::DSV> m_descriptor_heap{};

		//実際のリソース
		std::array<resource, Size> m_depth_resource{};

		//初期化の実装部分
		template<typename Head, typename ...Tail>
		bool depth_buffer_init_impl(device* device, unsigned int cnt, Head&& head, Tail&&... tail);

	public:
		//pairsは横と縦のSize個のstd::pair
		template<typename ...Pairs>
		bool initialize(device*,Pairs&&... pairs);

		//index番目の深度バッファのクリア
		void clear(command_list*, unsigned int index);

		//index番目のハンドルの取得
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(unsigned int index);
		D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_handle(unsigned int index);

		//index番目のリソースの取得
		resource* get_resource(unsigned int index);
	};



	template<unsigned int Size>
	template<typename Head, typename ...Tail>
	inline bool depth_buffer<Size>::depth_buffer_init_impl(device* device, unsigned int cnt, Head&& head, Tail&&... tail)
	{
		m_depth_resource[cnt] = create_depth_resource(device, head.first, head.second);

		if (m_depth_resource[cnt].is_empty()) {
			std::cout << "depth buffer init " << cnt << " is failed\n";
			return false;
		}

		m_descriptor_heap.create_view<create_view_type::DSV>(device, m_depth_resource[cnt].get());

		if constexpr (sizeof...(tail) <= 0)
			return true;
		else
			return depth_buffer_init_impl(device, cnt + 1, std::forward<Tail>(tail)...);
		
	}

	template<unsigned int Size>
	template<typename ...Pairs>
	inline bool depth_buffer<Size>::initialize(device* device, Pairs && ...pairs)
	{
		static_assert(sizeof...(pairs) == Size);

		if (!m_descriptor_heap.initialize(device, Size)) {
			std::cout << "depth descriptor heap init is failed\n";
			return false;
		}

		if constexpr (sizeof...(pairs) <= 0)
			return true;
		else
			return depth_buffer_init_impl(device, 0, std::forward<Pairs>(pairs)...);
	}

	template<unsigned int Size>
	inline void depth_buffer<Size>::clear(command_list* cl,unsigned int index)
	{
		cl->get()->ClearDepthStencilView(m_descriptor_heap.get_cpu_handle(index),
			D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
	}

	template<unsigned int Size>
	inline D3D12_GPU_DESCRIPTOR_HANDLE depth_buffer<Size>::get_gpu_handle(unsigned int index)
	{
		return m_descriptor_heap.get_gpu_handle(index);
	}

	template<unsigned int Size>
	inline D3D12_CPU_DESCRIPTOR_HANDLE depth_buffer<Size>::get_cpu_handle(unsigned int index)
	{
		return m_descriptor_heap.get_cpu_handle(index);
	}

	template<unsigned int Size>
	inline resource* depth_buffer<Size>::get_resource(unsigned int index)
	{
		return &m_depth_resource[index];
	}


}