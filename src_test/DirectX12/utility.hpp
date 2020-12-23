#pragma once
#include<type_traits>
#include<iostream>
#include<d3d12.h>
#include<dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace ichi
{

	//�o�b�t�@�ɏ����}�b�v
	//T�͔z�񂩃R���e�i
	template<typename T>
	bool map(ID3D12Resource* buffer, T&& t)
	{
		using value_type = std::remove_reference_t<decltype(t[0])>;

		value_type* target = nullptr;
		auto result = buffer->Map(0, nullptr, (void**)&target);
		//���s�����Ƃ�
		if (FAILED(result)) {
			std::cout << __func__ << " is failed\n";
			return false;
		}

		std::copy(std::begin(t), std::end(t), target);
		buffer->Unmap(0, nullptr);

		return true;
	}

}