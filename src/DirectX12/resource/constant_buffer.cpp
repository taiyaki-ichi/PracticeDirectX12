#include"constant_buffer.hpp"

namespace DX12
{
	bool constant_buffer::initialize(device* device, unsigned int size)
	{
		//�T�C�Y��16�̔{�����Ⴀ�Ȃ��Ƃ����Ȃ��̂ŃA���C�����g
		size = (size + 0xff) & ~0xff;

		return upload_resource::initialize(device, size);
	}
}