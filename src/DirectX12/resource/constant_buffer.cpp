#include"constant_buffer.hpp"

namespace DX12
{
	bool constant_buffer::initialize(device* device, unsigned int size)
	{
		//サイズは16の倍数じゃあないといけないのでアライメント
		size = (size + 0xff) & ~0xff;

		return upload_resource::initialize(device, size);
	}
}