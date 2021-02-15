#pragma once
#include"resource_base.hpp"
#include"map_function.hpp"

namespace DX12
{

	//�C���f�b�N�X�o�b�t�@��o�[�e�b�N�X�o�b�t�@�̂��ƂɂȂ�
	//���Ƃ�texture�̃A�b�v���[�h�p�Ƃ�
	//���̂܂܎g�����Ƃ͂Ȃ�����
	class upload_resource : public resource_base
	{
	public:
		upload_resource() = default;
		virtual ~upload_resource() = default;

		bool initialize(device*, unsigned int size);

		//resource�ւ̃}�b�v
		template<typename T>
		void map(T&&);
	};



	template<typename T>
	inline void upload_resource::map(T&& t)
	{
		map_to_resource(get(), std::forward<T>(t));
	}

}