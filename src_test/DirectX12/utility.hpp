#pragma once
#include<type_traits>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{

	//バッファに情報をマップ
	//Tは配列かコンテナ
	template<typename T>
	inline  bool map_func(ID3D12Resource* buffer, T&& t)
	{
		using value_type = std::remove_reference_t<decltype(t[0])>;

		value_type* target = nullptr;
		auto result = buffer->Map(0, nullptr, (void**)&target);
		//失敗したとき
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(t), std::end(t), target);
		buffer->Unmap(0, nullptr);

		return true;
	}

	//行列用
	template<>
	inline bool map_func<DirectX::XMMATRIX&>(ID3D12Resource* buffer, DirectX::XMMATRIX& m)
	{
		DirectX::XMMATRIX* ptr = nullptr;
		if (FAILED(buffer->Map(0, nullptr, (void**)&ptr))) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		*ptr = m;
		buffer->Unmap(0, nullptr);

		return true;
	}

}