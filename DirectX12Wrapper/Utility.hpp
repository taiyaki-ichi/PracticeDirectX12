#pragma once
#include<utility>

namespace DX12
{
	inline constexpr std::size_t AlignmentSize(std::size_t size, std::size_t alignment) {
		return size + alignment - size % alignment;
	}

}