#pragma once
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{

	//バッファに情報をマップ
	template<typename Value, size_t N>
	bool map(ID3D12Resource* buffer, const Value(&a)[N])
	{
		Value* target = nullptr;
		auto result = buffer->Map(0, nullptr, (void**)&target);
		//失敗したとき
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(a), std::end(a), target);
		buffer->Unmap(0, nullptr);

		return true;
	}

}