#pragma once
#include<type_traits>

namespace DX12
{
	class map_ostream
	{
		void* ptr;

	public:
		map_ostream(void*);

		template<typename T>
		map_ostream& operator<<(T const&);
	};

	class map_istream
	{
		void* ptr;

	public:
		map_istream(void*);

		template<typename T>
		map_istream& operator>>(T&);
	};

	//value_type‚ªFormat‚Ìê‡‚Ístream‚ğ
	//struct‚Ìê‡‚Ípointer‚ğ•Ô‚·
	template<typename T>
	auto map(T&);


	
	//
	//
	//

	map_ostream::map_ostream(void* p)
		:ptr{ p }
	{
	}

	template<typename T>
	inline map_ostream& map_ostream::operator<<(T const& in)
	{
		auto tmp = static_cast<std::remove_cv_t<T>*>(ptr);
		*tmp = in;
		ptr = reinterpret_cast<void*>(tmp + 1);
		return *this;
	}


	map_istream::map_istream(void* p)
		:ptr{ p }
	{
	}

	template<typename T>
	inline map_istream& map_istream::operator>>(T& out)
	{
		auto tmp = static_cast<std::remove_cv_t<T>*>(ptr);
		out = *tmp;
		ptr = reinterpret_cast<void*>(tmp + 1);
		return *this;
	}


	template<typename T>
	auto map(T& t)
	{
		static_assert(T::heap_property != resource_heap_property::DEFAULT);

		void* ptr = nullptr;
		t.get()->Map(0, nullptr, &ptr);

		if constexpr (is_format<typename T::value_type>::value || is_format_tuple<typename T::value_type>::value)
		{
			if constexpr (T::heap_property == resource_heap_property::UPLOAD)
				return map_ostream{ ptr };
			else if constexpr (T::heap_property == resource_heap_property::READ_BACK)
				return map_istream{ ptr };
		}
		else
		{
			if constexpr (T::heap_property == resource_heap_property::UPLOAD)
				return static_cast<typename T::value_type*>(ptr);
			else if constexpr (T::heap_property == resource_heap_property::READ_BACK)
				return static_cast<typename T::value_type const*>(ptr);
		}
	}

}