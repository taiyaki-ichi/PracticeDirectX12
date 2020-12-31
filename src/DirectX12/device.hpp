#pragma once
#include<type_traits>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{
	class command_list;
	class vertex_buffer;
	class index_buffer;
	class constant_buffer_resource;
	template<bool>
	class texture_shader_resource_base;
	class double_buffer;
	class pipeline_state;

	class device
	{
		ID3D12Device* m_device = nullptr;

		//deveice作るのに必要
		//6とか1とかの数字はいまいち理解していない
		IDXGIFactory5* m_factory = nullptr;
		IDXGIAdapter1* m_adaptor = nullptr;

	public:
		device() = default;
		~device();

		bool initialize();

		//deviceを使って作る系		
		template<typename T,typename... Args>
		T* create(Args&&... args) {
			//デフォルト構築が可能
			static_assert(std::is_default_constructible_v<T>);
			//メンバ関数にinitializeを持っておりその第一引数はdevice
			static_assert(std::is_invocable_v<decltype(&T::initialize), T, ichi::device*, Args...>);

			auto t = new T{};
			if (t->initialize(this, std::forward<Args>(args)...)) {
				return t;
			}
			else {
				std::cout << "device create is failed\n";
				delete t;
				return nullptr;
			}
		
		}
		
		ID3D12Device* get() const noexcept;

	};
}