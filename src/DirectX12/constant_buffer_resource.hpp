#pragma once
#include"utility.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class device;

	//定数バッファ
	//専用のディスクリプタヒープにViewを作る
	class constant_buffer_resource
	{
		ID3D12Resource* m_resource = nullptr;

	public:
		constant_buffer_resource() = default;
		~constant_buffer_resource();

		//device内で使用
		//アライメントは関数内でやる
		bool initialize(device*,unsigned int size);

		template<typename T>
		bool map(T&& t) {
			return map_func(m_resource, std::forward<T>(t));
		}

		template<typename Func,typename... Ts>
		bool map(Func&& mapFunc, Ts&&... ts) {
			return mapFunc(m_resource, std::forward<Ts>(ts)...);
		}

		//ディスクリプタヒープに関連づけるときに使用する
		ID3D12Resource* get() const noexcept;
	};
}