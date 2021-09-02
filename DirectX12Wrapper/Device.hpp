#pragma once
#include"utility.hpp"
#include<d3d12.h>
#include<dxgi1_6.h>
#include<array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{

	class device 
	{
		release_unique_ptr<ID3D12Device> device_ptr{};

		release_unique_ptr<IDXGIFactory5> factory_ptr{};
		release_unique_ptr<IDXGIAdapter1> adaptor_ptr{};

	public:
		device() = default;
		~device() = default;

		device(device&&) = default;
		device& operator=(device&&) = default;

		void initialize();

		ID3D12Device* get() const noexcept;
	};

	//
	//
	//

	inline void device::initialize()
	{
#ifdef _DEBUG
		ID3D12Debug* debugLayer = nullptr;
		if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
			THROW_EXCEPTION("");
		}
		else {
			debugLayer->EnableDebugLayer();
			debugLayer->Release();
		}
#endif
		IDXGIFactory5* f = nullptr;
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&f))))
			THROW_EXCEPTION("");
		factory_ptr.reset(f);

		UINT adapterIndex = 0;
		DXGI_ADAPTER_DESC1 desc{};
		IDXGIAdapter1* a = nullptr;
		while (true) {
			factory_ptr->EnumAdapters1(adapterIndex, &a);
			a->GetDesc1(&desc);

			//適切なアダプタが見つかった場合
			if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
				break;

			adapterIndex++;
			//見つからなかった場合
			if (adapterIndex == DXGI_ERROR_NOT_FOUND)
				THROW_EXCEPTION("");
		}
		adaptor_ptr.reset(a);


		std::array levels{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		ID3D12Device* d = nullptr;
		for (auto l : levels) {
			if (SUCCEEDED(D3D12CreateDevice(a, l, IID_PPV_ARGS(&d))))
				break;
			if (l == levels.back())
				THROW_EXCEPTION("");
		}
		device_ptr.reset(d);
	}

	inline ID3D12Device* device::get() const noexcept
	{
		return device_ptr.get();
	}
}