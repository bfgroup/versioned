// Copyright René Ferdinand Rivera Morell
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

#include <versioned/versioned.hpp>

#include <boost/core/lightweight_test.hpp>

struct V
{
	const char * i = nullptr;
	std::vector<const char *> p;
	template <class... T>
	V(const char * a, T... b)
		: i(a)
		, p({ b... })
	{}
};
int main()
{
	BOOST_TEST(true);
	{
		std::cout << "TEST: version_tag from_chars\n";
		V good_values[] = {
			{ "alpha", "alpha" },
			{ "alpha.1", "alpha", "1" },
			{ "alpha.beta", "alpha", "beta" },
			{ "beta", "beta" },
			{ "beta.2", "beta", "2" },
			{ "beta.11", "beta", "11" },
			{ "rc.1", "rc", "1" },
			{ "0.3.7", "0", "3", "7" },
			{ "x.7.z.92", "x", "7", "z", "92" },
			{ "x-y-z.--", "x-y-z", "--" },
		};
		for (auto & v : good_values)
		{
			std::cout << "> " << v.i << "\n";
			versioned::version_tag i;
			const char * first = v.i;
			const char * last = first + std::strlen(first);
			auto r = from_chars(first, last, i);
			BOOST_TEST_EQ(int(r.ec), int(std::errc {}));
			if (r.ec == std::errc {})
			{
				BOOST_TEST_EQ(r.ptr, last);
				BOOST_TEST_EQ(i.size(), v.p.size());
				std::size_t j = 0;
				for (auto c : v.p)
				{
					BOOST_TEST_EQ(c, i.at(j++));
				}
			}
		}
	}
	{
		std::cout << "TEST: version_tag from_chars (fail)\n";
		const char * bad_values[] = {
			"",
			"*1",
			"^&$e",
			"$95d",
			"#5",
			"#yz",
		};
		for (auto & v : bad_values)
		{
			std::cout << "> " << v << "\n";
			versioned::version_tag i;
			const char * first = v;
			const char * last = first + std::strlen(first);
			auto r = from_chars(first, last, i);
			BOOST_TEST_NE(int(r.ec), int(std::errc {}));
		}
	}
	{
		std::cout << "TEST: version_tag to_string\n";
		std::tuple<const char *, const char *> round_trip_values[] = {
			{ "a.1.x", "a.1.x" },
			{ "b.2.x*1", "b.2.x" },
			{ "c.3.x^&$e", "c.3.x" },
			{ "def.100.y$95d", "def.100.y" },
			{ "ghijklmop.9999.z#5", "ghijklmop.9999.z" },
			{ "rc.1#yz", "rc.1" },
		};
		for (auto & v : round_trip_values)
		{
			versioned::version_tag i;
			from_chars(std::get<0>(v),
				std::get<0>(v) + std::strlen(std::get<0>(v)), i);
			std::string s = to_string(i);
			BOOST_TEST_CSTR_EQ(std::get<1>(v), s.c_str());
		}
	}
	{
		std::cout << "TEST: version_tag compare\n";
		struct compare_val
		{
			const char * a;
			const char * b;
		};
		compare_val compare_values[] = {
			{ "1.0.0", "2.0.0" },
			{ "1.0.0", "1.0.1" },
			{ "1.0", "1.0.1" },
			{ "1", "1.0.1" },
			{ "1.0.0", "1.1" },
			{ "rc.1", "" },
		};
		for (auto & value : compare_values)
		{
			std::cout << "> " << value.a << " <=> " << value.b << "\n";
			{
				versioned::version_tag a;
				versioned::version_tag b;
				auto ar = from_chars(value.a, value.a + strlen(value.a), a);
				auto br = from_chars(value.b, value.b + strlen(value.b), b);
				if (!a.empty()) BOOST_TEST_EQ(int(ar.ec), int(std::errc {}));
				if (!b.empty()) BOOST_TEST_EQ(int(br.ec), int(std::errc {}));
				BOOST_TEST_NO_THROW(compare(a, b));
				BOOST_TEST_LT(compare(a, b), 0);
			}
		}
	}
	{
		std::cout << "TEST: version_tag sort (op==, op<)\n";
		const char * values[] = {
			"1.0.0",
			"1.0.1",
			"1.1",
			"1.1.1",
			"13.72.6",
			"2.0.0",
			"9.9.9",
			"99.99.99",
			"999.999.999",
			"9999.9999.9999",
			"99999.99999.99999",
			"999999.999999.999999",
			"9999999.9999999.9999999",
			"99999999.99999999.99999999",
			"alpha",
			"alpha.1",
			"alpha.beta",
			"beta",
			"beta.11",
			"beta.2",
			"rc.1",
		};
		std::vector<versioned::version_tag> base_vals;
		for (auto value : values)
		{
			versioned::version_tag a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			base_vals.push_back(a);
		}
		std::vector<versioned::version_tag> test_vals = base_vals;
		std::shuffle(test_vals.begin(), test_vals.end(),
			std::mt19937(versioned::detail::masked_max(291020249555917)));
		std::sort(test_vals.begin(), test_vals.end());
		for (std::size_t i = 0; i < base_vals.size(); ++i)
		{
			std::cout << "> " << to_string(base_vals[i])
					  << " == " << to_string(test_vals[i]) << "\n";
			BOOST_TEST(base_vals[i] == test_vals[i]);
		}
	}
	{
		std::cout << "TEST: version_tag hash\n";
		const char * values[] = {
			"1.0.0",
			"1.0.1",
			"1.1",
			"1.1.1",
			"13.72.6",
			"2.0.0",
			"9.9.9",
			"99.99.99",
			"999.999.999",
			"9999.9999.9999",
			"99999.99999.99999",
			"999999.999999.999999",
			"9999999.9999999.9999999",
			"99999999.99999999.99999999",
			"alpha",
			"alpha.1",
			"alpha.beta",
			"beta",
			"beta.11",
			"beta.2",
			"rc.1",
		};
		std::unordered_map<versioned::version_tag, const char *> m;
		for (auto value : values)
		{
			versioned::version_tag a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			std::cout << "> " << value
					  << std::string(40 - std::strlen(value), ' ')
					  << " hash == " << hash(a) << "\n";
			m[a] = value;
		}
		for (auto value : values)
		{
			std::cout << "> " << value << "\n";
			versioned::version_tag a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			BOOST_TEST_EQ(m[a], value);
		}
	}
	return boost::report_errors();
}
