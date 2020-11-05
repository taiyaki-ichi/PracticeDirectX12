#pragma once

namespace graphics
{
	size_t alignment_size(size_t size, size_t alignment) {
		return size + alignment - size % alignment;
	}
}