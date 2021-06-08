#pragma once

namespace DX12
{
	enum class VertexLayoutFormat {
		Float4,
		Float3,
		Float2
	};

	struct VertexLayout {
		const char* name{};
		VertexLayoutFormat format{};
	};

}