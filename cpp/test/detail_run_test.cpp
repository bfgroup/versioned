// Copyright René Ferdinand Rivera Morell
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include <versioned/versioned.hpp>

#include <boost/core/lightweight_test.hpp>

int main()
{
	BOOST_TEST(true);
	{
		using namespace ::bfg::versioned::detail;
		int v = 0;
		const char * s = "13.72.1";
		BOOST_TEST_EQ(from_chars_10(s, s + std::strlen(s), v), (s + 2));
		BOOST_TEST_EQ(v, 13);
	}
	struct V
	{
		const char * s = nullptr;
		int x = 0;
		int y = 0;
		int z = 0;
		V() = default;
		V(const char * s_, int x_, int y_, int z_)
			: s(s_)
			, x(x_)
			, y(y_)
			, z(z_)
		{}
	};
	V vs[] = {
		{ "1", 1, 0, 0 },
		{ "1.0", 1, 0, 0 },
		{ "1.0.0", 1, 0, 0 },
		{ "1.1.1", 1, 1, 1 },
		{ "9.9.9", 9, 9, 9 },
		{ "99.99.99", 99, 99, 99 },
		{ "999.999.999", 999, 999, 999 },
		{ "9999.9999.9999", 9999, 9999, 9999 },
		{ "99999.99999.99999", 99999, 99999, 99999 },
		{ "999999.999999.999999", 999999, 999999, 999999 },
		{ "9999999.9999999.9999999", 9999999, 9999999, 9999999 },
		{ "99999999.99999999.99999999", 99999999, 99999999, 99999999 },
		{ "13.72.7", 13, 72, 7 },
	};
	{
		using from_chars_f
			= const char * (*)(const char *, const char *, int &);
		auto check = [](const V & v, from_chars_f f) {
			V u;
			const char * first = v.s;
			const char * last = first + std::strlen(first);
			const char * p = f(first, last, u.x);
			const char * q = f(p + 1, last, u.y);
			const char * r = f(q + 1, last, u.z);
			BOOST_TEST_EQ(v.x, u.x);
			BOOST_TEST_EQ(v.y, u.y);
			BOOST_TEST_EQ(v.z, u.z);
			if (p < last) BOOST_TEST_EQ(*p, '.');
			if (q < last) BOOST_TEST_EQ(*q, '.');
			if (r < last) BOOST_TEST_EQ(*r, '.');
			if (p < last && q < last)
			{
				std::string s = std::to_string(u.x) + "." + std::to_string(u.y)
					+ "." + std::to_string(u.z);
				BOOST_TEST_CSTR_EQ(v.s, s.c_str());
			}
		};
		auto all = [&](const char * context, from_chars_f f) {
			std::cout << "TEST: " << context << "\n";
			for (auto & v : vs) check(v, f);
		};
		all("::bfg::versioned::detail::from_chars_10",
			&::bfg::versioned::detail::from_chars_10<int>);
	}
	{
		const std::size_t numbers[] = {
			0,
			1,
			2,
			9,
			99,
			999,
			1234567890,
		};
		for (auto n : numbers)
		{
			BOOST_TEST_EQ(
				std::to_string(n), ::bfg::versioned::detail::to_string_10(n));
		}
	}
	{
		std::cout << "TEST: hash_combine\n";
		const std::uint32_t numbers[] = {
			0,
			1,
			2,
			9,
			99,
			999,
			1234567890,
		};
		std::uint32_t ha = 0;
		std::uint32_t hb = 0;
		for (auto n : numbers)
		{
			ha = ::bfg::versioned::detail::hash_combine(ha, n);
			hb = ::bfg::versioned::detail::hash_combine(n, hb);
		}
		std::cout << "> ha = " << ha << ", hb = " << hb << "\n";
		BOOST_TEST_NE(ha, hb);
	}
	{
		const std::uint64_t numbers[] = {
			0,
			1,
			2,
			9,
			99,
			999,
			1234567890,
		};
		std::uint64_t ha = 0;
		std::uint64_t hb = 0;
		for (auto n : numbers)
		{
			ha = ::bfg::versioned::detail::hash_combine(ha, n);
			hb = ::bfg::versioned::detail::hash_combine(n, hb);
		}
		std::cout << "> ha = " << ha << ", hb = " << hb << "\n";
		BOOST_TEST_NE(ha, hb);
	}
	return boost::report_errors();
}
