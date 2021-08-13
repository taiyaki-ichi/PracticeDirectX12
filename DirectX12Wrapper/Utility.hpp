#pragma once
#include<utility>

namespace DX12
{
	template<typename T>
	inline constexpr T AlignmentSize(T size, T alignment) {
		return size + alignment - size % alignment;
	}

}