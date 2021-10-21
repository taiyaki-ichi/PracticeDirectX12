#pragma once
#include"../format.hpp"
#include<iterator>
#include<vector>
namespace DX12
{
	template<std::size_t I,typename FormatTuple>
	struct format_tuple_iterator
	{
		void* ptr = nullptr;

	public:
		format_tuple_iterator(void* p);

		using difference_type = std::ptrdiff_t;
		using value_type = typename convert_type<FormatTuple::template format_type<I>::component_type, FormatTuple::template format_type<I>::component_size>::type*;
		using reference = value_type&;
		using pointer = value_type*;

		using iterator_category = std::random_access_iterator_tag;

		reference operator*() noexcept;
		reference const operator*() const noexcept;

		format_tuple_iterator& operator++();
		format_tuple_iterator operator++(int);
		format_tuple_iterator& operator--();
		format_tuple_iterator operator--(int);

		format_tuple_iterator& operator+=(difference_type n);
		format_tuple_iterator operator+(difference_type n) const;
		template<std::size_t I, typename FormatTuple>
		friend format_tuple_iterator operator+(difference_type n, format_tuple_iterator<I, FormatTuple> const& i);

		format_tuple_iterator& operator-=(difference_type n);
		format_tuple_iterator operator-(difference_type n) const;
		template<std::size_t I, typename FormatTuple>
		friend format_tuple_iterator operator-(difference_type n, format_tuple_iterator<I, FormatTuple> const& i);

		template<std::size_t I, typename FormatTuple>
		friend typename format_tuple_iterator<I, FormatTuple>::difference_type operator-(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I, FormatTuple> const& r);

		template<std::size_t I, typename FormatTuple>
		friend bool operator==(format_tuple_iterator<I,FormatTuple> const& l,format_tuple_iterator<I,FormatTuple> const& r) noexcept;
		template<std::size_t I, typename FormatTuple>
		friend bool operator!=(format_tuple_iterator<I,FormatTuple> const& l,format_tuple_iterator<I,FormatTuple> const& r) noexcept;

		template<std::size_t I, typename FormatTuple>
		friend bool operator<(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I,FormatTuple> const& r);
		template<std::size_t I, typename FormatTuple>
		friend bool operator<=(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I,FormatTuple> const& r);
		template<std::size_t I, typename FormatTuple>
		friend bool operator>(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I,FormatTuple> const& r);
		template<std::size_t I, typename FormatTuple>
		friend bool operator>=(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I,FormatTuple> const& r);
	}; 

	//
	//
	//

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple>::format_tuple_iterator(void* p)
		:ptr{ p }
	{
	}

	template<std::size_t I, typename FormatTuple>
	inline typename format_tuple_iterator<I, FormatTuple>::reference format_tuple_iterator<I, FormatTuple>::operator*() noexcept
	{
		return reinterpret_cast<typename format_tuple_iterator<I, FormatTuple>::reference>(ptr);
	}

	template<std::size_t I, typename FormatTuple>
	inline typename format_tuple_iterator<I, FormatTuple>::reference const format_tuple_iterator<I, FormatTuple>::operator*() const noexcept
	{
		return reinterpret_cast<typename format_tuple_iterator<I, FormatTuple>::reference>(ptr);
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple>& format_tuple_iterator<I, FormatTuple>::operator++()
	{
		auto tmpPtr = reinterpret_cast<std::uint8_t*>(ptr) + FormatTuple::get_formats_stride();
		ptr = reinterpret_cast<void*>(tmpPtr);
		return *this;
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple> format_tuple_iterator<I, FormatTuple>::operator++(int)
	{
		auto tmp = *this;
		auto tmpPtr = reinterpret_cast<std::uint8_t*>(ptr) + FormatTuple::get_formats_stride();
		ptr = reinterpret_cast<void*>(tmpPtr);
		return tmp;
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple>& format_tuple_iterator<I, FormatTuple>::operator--()
	{
		auto tmpPtr = reinterpret_cast<std::uint8_t*>(ptr) - FormatTuple::get_formats_stride();
		ptr = reinterpret_cast<void*>(tmpPtr);
		return *this;
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple> DX12::format_tuple_iterator<I, FormatTuple>::operator--(int)
	{
		auto tmp = *this;
		auto tmpPtr = reinterpret_cast<std::uint8_t*>(ptr) - FormatTuple::get_formats_stride();
		ptr = reinterpret_cast<void*>(tmpPtr);
		return tmp;
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple>& DX12::format_tuple_iterator<I, FormatTuple>::operator+=(difference_type n)
	{
		auto tmpPtr = reinterpret_cast<std::uint8_t*>(ptr) + FormatTuple::get_formats_stride() * n;
		ptr = reinterpret_cast<void*>(tmpPtr);
		return *this;
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple> format_tuple_iterator<I, FormatTuple>::operator+(difference_type n) const
	{
		auto tmpPtr = reinterpret_cast<std::uint8_t*>(ptr) + FormatTuple::get_formats_stride() * n;
		return { tmpPtr };
	}

	template<std::size_t I, typename FormatTuple>
	format_tuple_iterator<I, FormatTuple> operator+(typename format_tuple_iterator<I, FormatTuple>::difference_type n, format_tuple_iterator<I, FormatTuple> const& i)
	{
		return { i + n };
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple>& format_tuple_iterator<I, FormatTuple>::operator-=(difference_type n)
	{
		auto tmpPtr = reinterpret_cast<std::uint8_t*>(ptr) - FormatTuple::get_formats_stride() * n;
		ptr = reinterpret_cast<void*>(tmpPtr);
		return *this;
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple> format_tuple_iterator<I, FormatTuple>::operator-(difference_type n) const
	{
		auto tmpPtr = reinterpret_cast<std::uint8_t*>(ptr) - FormatTuple::get_formats_stride() * n;
		return { tmpPtr };
	}

	template<std::size_t I, typename FormatTuple>
	inline format_tuple_iterator<I, FormatTuple> operator-(typename format_tuple_iterator<I, FormatTuple>::difference_type n, format_tuple_iterator<I, FormatTuple> const& i)
	{
		return { i - n };
	}

	template<std::size_t I, typename FormatTuple>
	inline typename format_tuple_iterator<I, FormatTuple>::difference_type operator-(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I, FormatTuple> const& r)
	{
		return (static_cast<std::uint8_t*>(l .ptr) - static_cast<std::uint8_t*>(r.ptr)) / FormatTuple::get_formats_stride();
	}


	template<std::size_t I, typename FormatTuple>
	inline bool operator==(format_tuple_iterator<I, FormatTuple> const& l, format_tuple_iterator<I, FormatTuple> const& r) noexcept
	{
		return l.ptr == r.ptr;
	}

	template<std::size_t I, typename FormatTuple>
	inline bool operator!=(format_tuple_iterator<I, FormatTuple> const& l, format_tuple_iterator<I, FormatTuple> const& r) noexcept
	{
		return l.ptr != r.ptr;
	}

	template<std::size_t I, typename FormatTuple>
	inline bool operator<(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I,FormatTuple> const& r)
	{
		return l.ptr < r.ptr;
	}

	template<std::size_t I, typename FormatTuple>
	inline bool operator<=(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I,FormatTuple> const& r)
	{
		return l.ptr <= r.ptr;
	}

	template<std::size_t I, typename FormatTuple>
	inline bool operator>(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I,FormatTuple> const& r)
	{
		return l.ptr > r.ptr;
	}

	template<std::size_t I, typename FormatTuple>
	inline bool operator>=(format_tuple_iterator<I,FormatTuple> const& l, format_tuple_iterator<I,FormatTuple> const& r)
	{
		return l.ptr >= r.ptr;
	}


}