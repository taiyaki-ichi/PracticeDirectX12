#pragma once
#include"resource_base.hpp"
#include"map_function.hpp"

namespace DX12
{

	//インデックスバッファやバーテックスバッファのもとになる
	//あとはtextureのアップロード用とか
	//そのまま使うことはなさそう
	class upload_resource : public resource_base
	{
	public:
		upload_resource() = default;
		virtual ~upload_resource() = default;

		bool initialize(device*, unsigned int size);

		//resourceへのマップ
		template<typename T>
		void map(T&&);
	};



	template<typename T>
	inline void upload_resource::map(T&& t)
	{
		map_to_resource(get(), std::forward<T>(t));
	}

}