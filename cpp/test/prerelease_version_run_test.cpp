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
		std::cout << "TEST: prerelease_version<int> from_chars\n";
		std::tuple<const char *, int> number_values[] = {
			{ "a.1.x", 1 },
			{ "b.2.x", 2 },
			{ "c.3.x", 3 },
			{ "def.100.y", 100 },
			{ "ghijklmop.9999.z", 9999 },
		};
		for (auto & v : number_values)
		{
			std::cout << "> " << std::get<0>(v) << "\n";
			versioned::prerelease_version<int> i;
			const char * first = std::get<0>(v);
			const char * last = first + std::strlen(first);
			auto r = from_chars(first, last, i);
			BOOST_TEST_EQ(int(r.ec), int(std::errc {}));
			BOOST_TEST(i.is_number_at(1));
			BOOST_TEST_EQ(std::get<1>(v), i.number_at(1));
			BOOST_TEST_THROWS(i.number_at(0), std::invalid_argument);
			BOOST_TEST_THROWS(i.at(3), std::out_of_range);
		}
	}
	{
		std::cout << "TEST: prerelease_version<int> to_string\n";
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
			versioned::prerelease_version<int> i;
			from_chars(std::get<0>(v),
				std::get<0>(v) + std::strlen(std::get<0>(v)), i);
			std::string s = to_string(i);
			BOOST_TEST_CSTR_EQ(std::get<1>(v), s.c_str());
		}
	}
	{
		std::cout << "TEST: prerelease_version<int> to_string\n";
		const char * values[] = {
			"1.0.0",
			"2.0.0",
			"1.0.0",
			"1.0.1",
			"1.0",
			"1.0.1",
			"1",
			"1.0.1",
			"1.0.0",
			"1.1",
		};
		for (auto value : values)
		{
			std::cout << "> " << value << "\n";
			{
				versioned::prerelease_version<int> a;
				BOOST_TEST_EQ(
					int(from_chars(value, value + strlen(value), a).ec),
					int(std::errc {}));
				BOOST_TEST_EQ(a.size(),
					std::count(value, value + strlen(value), '.') + 1);
				BOOST_TEST_EQ(to_string(a), value);
				for (std::size_t j = 0; j < a.size(); ++j)
				{
					BOOST_TEST(a.is_number_at(j));
					auto r = a.range_at(j);
					const char * z = std::get<0>(a.range_at(0));
					const char * b = std::get<0>(r);
					const char * e = std::get<1>(r);
					auto count = e - b;
					std::string v_sub(value + (b - z), count);
					std::string j_sub(b, count);
					std::cout << "  [" << j << "] " << j_sub << "\n";
					BOOST_TEST_EQ(v_sub, j_sub);
					auto n = std::strtoull(b, const_cast<char **>(&e), 10);
					BOOST_TEST_EQ(a.number_at(j), n);
				}
			}
		}
	}
	{
		std::cout << "TEST: prerelease_version<int> compare\n";
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
				versioned::prerelease_version<int> a;
				versioned::prerelease_version<int> b;
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
		std::cout << "TEST: prerelease_version<int> sort (op==, op<)\n";
		const char * values[] = {
			"1.0.0",
			"1.0.1",
			"1.1",
			"1.1.1",
			"2.0.0",
			"9.9.9",
			"13.72.6",
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
			"beta.2",
			"beta.11",
			"rc.1",
		};
		std::vector<versioned::prerelease_version<int>> base_vals;
		for (auto value : values)
		{
			versioned::prerelease_version<int> a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			base_vals.push_back(a);
		}
		std::vector<versioned::prerelease_version<int>> test_vals = base_vals;
		std::shuffle(
			test_vals.begin(), test_vals.end(), std::mt19937(291020249555917));
		std::sort(test_vals.begin(), test_vals.end());
		for (std::size_t i = 0; i < base_vals.size(); ++i)
		{
			std::cout << "> " << to_string(base_vals[i])
					  << " == " << to_string(test_vals[i]) << "\n";
			BOOST_TEST(base_vals[i] == test_vals[i]);
		}
	}
	{
		std::cout << "TEST: prerelease_version<int> hash\n";
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
		std::unordered_map<versioned::prerelease_version<int>, const char *> m;
		for (auto value : values)
		{
			versioned::prerelease_version<int> a;
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
			versioned::prerelease_version<int> a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			BOOST_TEST_EQ(m[a], value);
		}
	}
	return boost::report_errors();
}
