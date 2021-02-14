#pragma once
#include<type_traits>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	class command_list;
	class vertex_buffer;
	class index_buffer;
	class constant_buffer_resource;
	template<bool>
	class texture_shader_resource_base;
	class double_buffer;

	class device
	{
		ID3D12Device* m_device = nullptr;

		//deveiceì‚é‚Ì‚É•K—v
		//6‚Æ‚©1‚Æ‚©‚Ì”š‚Í‚¢‚Ü‚¢‚¿—‰ğ‚µ‚Ä‚¢‚È‚¢
		IDXGIFactory5* m_factory = nullptr;
		IDXGIAdapter1* m_adaptor = nullptr;

	public:
		device() = default;
		~device();

		bool initialize();
		
		ID3D12Device* get() const noexcept;

	};
}