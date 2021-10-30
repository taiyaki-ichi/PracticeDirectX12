#pragma once
#include<type_traits>

namespace DX12
{
	class map_ostream
	{
		void* ptr;

	public:
		map_ostream(void*);
		virtual ~map_ostream() = default;

		map_ostream(map_ostream&) = default;
		map_ostream& operator=(map_ostream&) = default;

		map_ostream(map_ostream&&) = default;
		map_ostream& operator=(map_ostream&&) = default;

		template<typename T>
		map_ostream& operator<<(T const&);
	};

	class map_istream
	{
		void* ptr;

	public:
		map_istream(void*);
		virtual ~map_istream() = default;

		map_istream(map_istream&) = default;
		map_istream& operator=(map_istream&) = default;

		map_istream(map_istream&&) = default;
		map_istream& operator=(map_istream&&) = default;

		template<typename T>
		map_istream& operator>>(T&);
	};

	//‰¼
	template<typename T>
	map_ostream map(T&);
	
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
	map_ostream map(T& t)
	{
		void* ptr = nullptr;
		t.get()->Map(0, nullptr, &ptr);
		return { ptr };
	}
}