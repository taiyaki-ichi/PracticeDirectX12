#pragma once
#include<utility>

namespace DX12
{
	inline constexpr std::uint32_t AlignmentSize(std::uint32_t size, std::uint32_t alignment) {
		return size + alignment - size % alignment;
	}

}