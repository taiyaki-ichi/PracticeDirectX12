#pragma once
#include<utility>
#include<memory>
#include<stdexcept>
#include<sstream>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace DX12
{
	template<typename T>
	inline constexpr T alignment(T size, T alignment) {
		return size + alignment - size % alignment;
	}

	template<typename T>
	inline constexpr T texture_data_pitch_alignment(T width) {
		return alignment<T>(width, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	}


	template<typename T>
	struct release_deleter {
		void operator()(T* ptr) {
			ptr->Release();
		}
	};

	template<typename T>
	using release_unique_ptr = std::unique_ptr<T, release_deleter<T>>;


	inline void throw_exception(char const* fileName,int line,char const* func,char const* str)
	{
		std::stringstream ss{};
		ss << fileName << " , " << line << " , " << func << " : " << str << "\n";
		throw std::runtime_error{ ss.str() };
	}

#define THROW_EXCEPTION(s)	throw_exception(__FILE__,__LINE__,__func__,s);


	//I”Ô–Ú‚ÌŒ^‚ðŽæ“¾
	template<std::size_t I,typename Head,typename... Tails>
	struct index_element {
		using type = typename index_element<I - 1, Tails...>::type;
	};

	template<typename Head,typename... Tails>
	struct index_element<0,Head,Tails...> {
		using type = Head;
	};
}