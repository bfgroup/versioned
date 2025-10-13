// Copyright René Ferdinand Rivera Morell
// Distributed under the Boost Software License, Version 1.0. (See
// versioned_narrowc_accumulateompanying file LICENSE.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef VERSIONED_VERSIONED_HPP
#define VERSIONED_VERSIONED_HPP

#include <algorithm>
#include <limits>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <vector>

namespace versioned { namespace detail {

// The decimal digits.
inline constexpr bool is_digit(const char c) { return c >= '0' && c <= '9'; }

// Alphanumeric, for identifiers.
inline constexpr bool is_alphanum(const char c)
{
	return is_digit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Identifer allowed character.
inline constexpr bool is_ident(const char c)
{
	return is_alphanum(c) || (c == '-');
}

// Base 10 only positive integer parsing. Believe it or not, this rather
// simple code provides the fastest throughput for parsing sequential
// integers.
template <typename I>
const char * from_chars_10(const char * first, const char * last, I & value)
{
	I e = 0;
	while (first < last && is_digit(*first)) e = e * 10 + (*(first++) - '0');
	value = e;
	return first;
}

// Compare, as ASCII, two string ranges.
inline int chars_cmp(
	const char * a0, const char * an, const char * b0, const char * bn)
{
	while ((a0 < an) && (b0 < bn) && (*a0 == *b0))
	{
		a0 += 1;
		b0 += 1;
	}
	return *a0 - *b0;
}

template <typename N, typename U>
constexpr N masked_max(U value)
{
	return value & static_cast<U>(::std::numeric_limits<N>::max());
}

// Simple hash values combine.
template <class T, class... N>
T hash_combine(T seed, N... an)
{
	T args[] = { an... };
	// Init.
	T result = seed;
	// Combine.
	for (auto a : args)
	{
		result ^= a + masked_max<T>(3772387269305686495)
			+ (result << (30 * 2 / sizeof(T)))
			+ (result >> (13 * 2 / sizeof(T)));
	}
	// Mix.
	result ^= (result >> (16 * 2 / sizeof(T)));
	result *= masked_max<T>(448100074733706);
	result ^= (result << (8 * 2 / sizeof(T)));
	result += masked_max<T>(190056597654806);
	result ^= (result >> (11 * 2 / sizeof(T)));
	return result;
}

// Backfill to_string as it's missing on some runtimes.
inline ::std::string to_string_10(::std::size_t value)
{
	::std::string result;
	result.reserve(::std::numeric_limits<::std::size_t>::max_digits10);
	do
	{
		result += '0' + (value % 10);
		value /= 10;
	}
	while (value > 0);
	::std::reverse(result.begin(), result.end());
	return result;
}

}} // namespace versioned::detail

namespace versioned {
struct from_chars_result
{
	const char * ptr;
	::std::errc ec;

	constexpr explicit operator bool() const noexcept
	{
		return ec == ::std::errc {};
	}

	friend constexpr bool operator==(
		const from_chars_result & a, const from_chars_result & b)
	{
		return (a.ptr == b.ptr) && (a.ec == b.ec);
	}
};
} // namespace versioned

/* tag::reference[]
== `versioned::version_core`
end::reference[] */

namespace versioned {
template <class Number, ::std::size_t Count = 3>
class version_core
{
	static_assert(::std::is_integral<Number>::value,
		"Version element type is not an integral type.");
	static_assert(Count > 0, "Version must contain at least one part.");

	public:
	using element_t = Number;
	static constexpr ::std::size_t element_c = Count;

	version_core() = default;
	version_core(version_core &&) = default;
	version_core(const version_core &) = default;
	version_core & operator=(const version_core &) = default;

	template <class... I>
	explicit version_core(element_t a0, I... an)
	{
		element_t args[] = { a0, an... };
		::std::size_t i = 0;
		for (auto a : args) number[i++] = a;
	}

	element_t & at(::std::size_t i)
	{
		if (i >= element_c)
			throw ::std::out_of_range(
				"No such component: " + detail::to_string_10(i));
		return number[i];
	}

	const element_t & at(::std::size_t i) const
	{
		if (i >= element_c)
			throw ::std::out_of_range(
				"No such component: " + detail::to_string_10(i));
		return number[i];
	}

	private:
	element_t number[element_c] = {};

	template <class N, ::std::size_t C>
	friend from_chars_result from_chars(
		const char * first, const char * last, version_core<N, C> & value);

	template <class N, ::std::size_t C>
	friend ::std::string to_string(const version_core<N, C> & value);

	template <class N0, ::std::size_t C0, class N1, ::std::size_t C1>
	friend int compare(
		const version_core<N0, C0> & a, const version_core<N1, C1> & b);

	template <class N0, ::std::size_t C0, class N1, ::std::size_t C1>
	friend bool operator==(
		const version_core<N0, C0> & a, const version_core<N1, C1> & b);

	template <class N0, ::std::size_t C0, class N1, ::std::size_t C1>
	friend bool operator<(
		const version_core<N0, C0> & a, const version_core<N1, C1> & b);

	template <class N, ::std::size_t C>
	friend ::std::size_t hash(const version_core<N, C> & value);
};

template <class N, ::std::size_t C>
from_chars_result from_chars(
	const char * first, const char * last, version_core<N, C> & value)
{
	if (first == last || !first || !last)
		return from_chars_result { first, ::std::errc::invalid_argument };

	from_chars_result result { first, ::std::errc(0) };
	typename version_core<N, C>::element_t
		number[version_core<N, C>::element_c] {};

	for (::std::size_t i = 0; i < version_core<N, C>::element_c; ++i)
	{
		result.ptr = detail::from_chars_10(result.ptr, last, number[i]);
		if (result.ptr == last || *result.ptr != '.') break;
		result.ptr += 1;
	}

	for (::std::size_t i = 0; i < version_core<N, C>::element_c; ++i)
		value.number[i] = number[i];

	return result;
}

template <class N, ::std::size_t C>
::std::string to_string(const version_core<N, C> & value)
{
	::std::string result;
	for (::std::size_t i = version_core<N, C>::element_c; i > 0; --i)
	{
		::std::size_t n = i - 1;
		if (n == 0)
			result = detail::to_string_10(value.number[n]) + result;
		else if (value.number[n] > 0)
			result = "." + detail::to_string_10(value.number[n]) + result;
	}
	return result;
}

template <class N0, ::std::size_t C0, class N1, ::std::size_t C1>
int compare(const version_core<N0, C0> & a, const version_core<N1, C1> & b)
{
	for (::std::size_t i = 0; i < version_core<N0, C0>::element_c
		&& i < version_core<N1, C1>::element_c;
		++i)
	{
		auto c = a.number[i]
			- static_cast<typename version_core<N0, C0>::element_t>(
				b.number[i]);
		if (c != 0) return int(c);
	}
	return int(version_core<N0, C0>::element_c)
		- int(version_core<N1, C1>::element_c);
}

template <class N0, ::std::size_t C0, class N1, ::std::size_t C1>
bool operator==(const version_core<N0, C0> & a, const version_core<N1, C1> & b)
{
	return compare(a, b) == 0;
}

template <class N0, ::std::size_t C0, class N1, ::std::size_t C1>
bool operator<(const version_core<N0, C0> & a, const version_core<N1, C1> & b)
{
	return compare(a, b) < 0;
}

template <class N, ::std::size_t C>
::std::size_t hash(const version_core<N, C> & value)
{
	::std::hash<typename version_core<N, C>::element_t> h {};
	::std::size_t result = 0;
	for (auto n : value.number) result = detail::hash_combine(result, h(n));
	return result;
}
} // namespace versioned

namespace std {
template <class N, ::std::size_t C>
struct hash<::versioned::version_core<N, C>>
{
	::std::size_t operator()(
		const ::versioned::version_core<N, C> & value) const noexcept
	{
		return ::versioned::hash(value);
	}
};
} // namespace std

/* tag::reference[]
== `versioned::version_tag`
end::reference[] */

namespace versioned {
class version_tag
{
	public:
	using element_t = ::std::string;
	using range_element_t = ::std::tuple<const char *, const char *>;

	version_tag() = default;
	version_tag(version_tag &&) = default;
	version_tag(const version_tag &) = default;
	version_tag & operator=(const version_tag &) = default;

	template <class... S>
	explicit version_tag(const char * a0, S... an)
	{
		info_ += a0;
		parts_.emplace_back(info_.size());
		const char * args[] = { an... };
		for (auto a : args)
		{
			info_ += ".";
			info_ += a;
			parts_.emplace_back(info_.size());
		}
	}

	template <class... S>
	explicit version_tag(const ::std::string & a0, S... an)
	{
		info_ += a0;
		parts_.emplace_back(info_.size());
		const char * args[] = { an... };
		for (auto a : args)
		{
			info_ += ".";
			info_ += a;
			parts_.emplace_back(info_.size());
		}
	}

	element_t at(::std::size_t i) const
	{
		auto r = this->range_at(i);
		return ::std::string(::std::get<0>(r), ::std::get<1>(r));
	}

	range_element_t range_at(::std::size_t i) const
	{
		if (i >= parts_.size())
			throw ::std::out_of_range(
				"No such component: " + detail::to_string_10(i));
		return ::std::make_tuple(
			info_.c_str() + (i == 0 ? 0 : parts_[i - 1] + 1),
			info_.c_str() + parts_[i]);
	}

	bool empty() const { return info_.empty(); }

	::std::size_t size() const { return parts_.size(); }

	private:
	// The info contains the part that is parsed from any original data.
	::std::string info_;
	// The parts hold the index to the end of each parsed segment.
	::std::vector<::std::size_t> parts_;

	friend from_chars_result from_chars(
		const char * first, const char * last, version_tag & value);

	friend ::std::string to_string(const version_tag & value);

	friend int compare(const version_tag & a, const version_tag & b);

	friend bool operator==(const version_tag & a, const version_tag & b);
	friend bool operator<(const version_tag & a, const version_tag & b);

	friend ::std::size_t hash(const version_tag & value);
};

inline from_chars_result from_chars(
	const char * first, const char * last, version_tag & value)
{
	if (first == last)
		return from_chars_result { first, ::std::errc::invalid_argument };

	from_chars_result result {};
	::std::vector<::std::size_t> parts;

	for (auto begin = first; begin < last;)
	{
		auto end = ::std::find_if(begin, last,
			[](char c) { return c == '.' || !detail::is_ident(c); });
		if (end == first)
		{
			// No valid info parts found. Indicate an error, as nothing is
			// not an allowed input.
			result.ptr = end;
			result.ec = ::std::errc::invalid_argument;
			break;
		}
		else if (end == last || *end == '.')
		{
			// Found the EOS or a part separator. We have a part. And may
			// have more.
			parts.emplace_back(end - first);
			begin = end + 1;
		}
		else if (!detail::is_ident(*end))
		{
			// We parsed a valid part, but reached the end of the valid
			// characters.
			parts.emplace_back(end - first);
			break;
		}
		else
		{
			// Should never happen. But check, and report an error if it
			// does.
			result.ptr = nullptr;
			result.ec = ::std::errc::invalid_argument;
			break;
		}
	}

	if (result.ec == ::std::errc {})
	{
		result.ptr = first + parts.back();
		value.info_ = ::std::string(first, result.ptr);
		value.parts_ = parts;
	}
	return result;
}

inline ::std::string to_string(const version_tag & value)
{
	return value.info_;
}

inline int compare(const version_tag & a, const version_tag & b)
{
	if (!a.empty() && b.empty()) return -1;
	if (a.empty() && !b.empty()) return 1;
	for (::std::size_t i = 0; i < a.size() && i < b.size(); ++i)
	{
		auto a_chars = a.range_at(i);
		auto b_chars = b.range_at(i);
		auto c
			= detail::chars_cmp(::std::get<0>(a_chars), ::std::get<1>(a_chars),
				::std::get<0>(b_chars), ::std::get<1>(b_chars));
		if (c != 0) return c;
	}
	return int(a.size()) - int(b.size());
}

inline bool operator==(const version_tag & a, const version_tag & b)
{
	return compare(a, b) == 0;
}

inline bool operator<(const version_tag & a, const version_tag & b)
{
	return compare(a, b) < 0;
}

inline ::std::size_t hash(const version_tag & value)
{
	return ::std::hash<decltype(value.info_)> {}(value.info_);
}
} // namespace versioned

namespace std {
template <>
struct hash<::versioned::version_tag>
{
	::std::size_t operator()(
		const ::versioned::version_tag & value) const noexcept
	{
		return ::versioned::hash(value);
	}
};
} // namespace std

/* tag::reference[]
== `versioned::prerelease_version`
end::reference[] */

namespace versioned {
template <class Number>
class prerelease_version : public version_tag
{
	public:
	using number_element_t = Number;

	number_element_t number_at(::std::size_t i) const
	{
		number_element_t n {};
		if (!is_number_at(i, n))
			throw ::std::invalid_argument("Element is not a number.");
		return n;
	}

	bool is_number_at(::std::size_t i) const
	{
		number_element_t n {};
		return is_number_at(i, n);
	}

	private:
	bool is_number_at(::std::size_t i, number_element_t & n) const
	{
		auto r = this->range_at(i);
		const char * e
			= detail::from_chars_10(::std::get<0>(r), ::std::get<1>(r), n);
		return (e == ::std::get<1>(r));
	}

	template <class N0, class N1>
	friend int compare(
		const prerelease_version<N0> & a, const prerelease_version<N1> & b);

	template <class N0, class N1>
	friend bool operator==(
		const prerelease_version<N0> & a, const prerelease_version<N1> & b);

	template <class N0, class N1>
	friend bool operator<(
		const prerelease_version<N0> & a, const prerelease_version<N1> & b);

	template <class N>
	::std::size_t hash(const prerelease_version<N> & value);
};

template <class N0, class N1>
int compare(const prerelease_version<N0> & a, const prerelease_version<N1> & b)
{
	if (!a.empty() && b.empty()) return -1;
	if (a.empty() && !b.empty()) return 1;
	for (::std::size_t i = 0; i < a.size() && i < b.size(); ++i)
	{
		typename prerelease_version<N0>::number_element_t a_n {};
		bool a_is_num = a.is_number_at(i, a_n);
		typename prerelease_version<N1>::number_element_t b_n {};
		bool b_is_num = b.is_number_at(i, b_n);
		if (a_is_num && b_is_num)
		{
			auto b_e = static_cast<
				typename prerelease_version<N0>::number_element_t>(b_n);
			if (a_n < b_e) return -1;
			if (b_e < a_n) return 1;
		}
		else if (a_is_num)
			return -1;
		else if (b_is_num)
			return 1;
		else
		{
			auto a_chars = a.range_at(i);
			auto b_chars = b.range_at(i);
			auto c = detail::chars_cmp(::std::get<0>(a_chars),
				::std::get<1>(a_chars), ::std::get<0>(b_chars),
				::std::get<1>(b_chars));
			if (c != 0) return c;
		}
	}
	return int(a.size()) - int(b.size());
}

template <class N0, class N1>
bool operator==(
	const prerelease_version<N0> & a, const prerelease_version<N1> & b)
{
	return compare(a, b) == 0;
}

template <class N0, class N1>
bool operator<(
	const prerelease_version<N0> & a, const prerelease_version<N1> & b)
{
	return compare(a, b) < 0;
}

template <class N>
::std::size_t hash(const prerelease_version<N> & value)
{
	return hash(static_cast<const version_tag &>(value));
}
} // namespace versioned

namespace std {
template <class N>
struct hash<::versioned::prerelease_version<N>>
{
	::std::size_t operator()(
		const ::versioned::prerelease_version<N> & value) const noexcept
	{
		return ::versioned::hash(value);
	}
};
} // namespace std

/* tag::reference[]
== `versioned::build_metadata`
end::reference[] */

namespace versioned {
class build_metadata : public version_tag
{};
} // namespace versioned

namespace std {
}

namespace versioned {
template <class Number = int,
	class Prerelease = prerelease_version<Number>,
	class Build = build_metadata>
class semver
{
	public:
	using version_t = version_core<Number, 3>;
	using prerelease_t = Prerelease;
	using build_t = Build;

	const version_t & version() const { return core_; }
	const prerelease_t & prerelease() const { return pre_; }
	const build_t & build() const { return build_; }

	version_t & version() { return core_; }
	prerelease_t & prerelease() { return pre_; }
	build_t & build() { return build_; }

	typename version_t::element_t major() const { return core_.at(0); }
	typename version_t::element_t minor() const { return core_.at(1); }
	typename version_t::element_t patch() const { return core_.at(2); }

	private:
	version_t core_;
	prerelease_t pre_;
	build_t build_;

	template <class N, class P, class B>
	friend from_chars_result from_chars(
		const char * first, const char * last, semver<N, P, B> & value);

	template <class N, class P, class B>
	friend ::std::string to_string(const semver<N, P, B> & value);

	template <class N0, class P0, class B0, class N1, class P1, class B1>
	friend int compare(
		const semver<N0, P0, B0> & a, const semver<N1, P1, B1> & b);

	template <class N0, class P0, class B0, class N1, class P1, class B1>
	friend bool operator==(
		const semver<N0, P0, B0> & a, const semver<N1, P1, B1> & b);

	template <class N0, class P0, class B0, class N1, class P1, class B1>
	friend bool operator<(
		const semver<N0, P0, B0> & a, const semver<N1, P1, B1> & b);

	template <class N, class P, class B>
	friend ::std::size_t hash(const semver<N, P, B> & value);
};

template <class N, class P, class B>
from_chars_result from_chars(
	const char * first, const char * last, semver<N, P, B> & value)
{
	if (first == last)
		return from_chars_result { first, ::std::errc::invalid_argument };

	from_chars_result result {};

	typename semver<N, P, B>::version_t v;
	{
		result = from_chars(first, last, v);
		if (result.ec != ::std::errc {}) return result;
		first = result.ptr;
	}

	typename semver<N, P, B>::prerelease_t p;
	if (first != last && *first == '-')
	{
		result = from_chars(first + 1, last, p);
		if (result.ec != ::std::errc {}) return result;
		first = result.ptr;
	}

	typename semver<N, P, B>::build_t b;
	if (first != last && *first == '+')
	{
		result = from_chars(first + 1, last, b);
		if (result.ec != ::std::errc {}) return result;
		first = result.ptr;
	}

	value.core_ = v;
	value.pre_ = p;
	value.build_ = b;
	return result;
}

template <class N, class P, class B>
::std::string to_string(const semver<N, P, B> & value)
{
	::std::string result = to_string(value.core_);
	if (!value.pre_.empty()) result += "-" + to_string(value.pre_);
	if (!value.build_.empty()) result += "+" + to_string(value.build_);
	return result;
}

template <class N0, class P0, class B0, class N1, class P1, class B1>
int compare(const semver<N0, P0, B0> & a, const semver<N1, P1, B1> & b)
{
	auto x = compare(a.core_, b.core_);
	if (x == 0) return compare(a.pre_, b.pre_);
	return x;
}

template <class N0, class P0, class B0, class N1, class P1, class B1>
bool operator==(const semver<N0, P0, B0> & a, const semver<N1, P1, B1> & b)
{
	return compare(a, b) == 0;
}

template <class N0, class P0, class B0, class N1, class P1, class B1>
bool operator<(const semver<N0, P0, B0> & a, const semver<N1, P1, B1> & b)
{
	return compare(a, b) < 0;
}

template <class N, class P, class B>
::std::size_t hash(const semver<N, P, B> & value)
{
	return detail::hash_combine(versioned::hash(value.core_),
		versioned::hash(value.pre_), versioned::hash(value.build_));
}
} // namespace versioned

namespace std {
template <class N, class P, class B>
struct hash<::versioned::semver<N, P, B>>
{
	::std::size_t operator()(
		const ::versioned::semver<N, P, B> & value) const noexcept
	{
		return ::versioned::hash(value);
	}
};
} // namespace std

#endif
