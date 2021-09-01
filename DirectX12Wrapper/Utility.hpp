#pragma once
#include<utility>
#include<memory>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	template<typename T>
	inline constexpr T Alignment(T size, T alignment) {
		return size + alignment - size % alignment;
	}

	template<typename T>
	inline constexpr T TextureDataPitchAlignment(T width) {
		return Alignment<T>(width, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	}


	template<typename T>
	struct release_deleter {
		void operator()(T* ptr) {
			ptr->Release();
		}
	};

	template<typename T>
	using release_unique_ptr = std::unique_ptr<T, release_deleter<T>>;
	
}