// Copyright René Ferdinand Rivera Morell
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <unordered_map>

#include <versioned/versioned.hpp>

#include <boost/core/lightweight_test.hpp>

int main()
{
	BOOST_TEST(true);
	{
		std::cout << "TEST: version_core<int> from_chars\n";
		struct good_value
		{
			const char * s;
			int x;
			int y;
			int z;
		};
		good_value good_values[] = {
			{ "0", 0, 0, 0 },
			{ "0.0", 0, 0, 0 },
			{ "0.0.0", 0, 0, 0 },
			{ "1", 1, 0, 0 },
			{ "1.1", 1, 1, 0 },
			{ "1.1.1", 1, 1, 1 },
			{ "1.0.0", 1, 0, 0 },
			{ "9.9.9", 9, 9, 9 },
			{ "99.99.99", 99, 99, 99 },
			{ "999.999.999", 999, 999, 999 },
			{ "9999.9999.9999", 9999, 9999, 9999 },
			{ "99999.99999.99999", 99999, 99999, 99999 },
			{ "999999.999999.999999", 999999, 999999, 999999 },
			{ "9999999.9999999.9999999", 9999999, 9999999, 9999999 },
			{ "99999999.99999999.99999999", 99999999, 99999999, 99999999 },
			{ "13.72.6", 13, 72, 6 },
		};
		for (auto & value : good_values)
		{
			std::cout << "> " << value.s << "\n";
			versioned::version_core<int> version;
			const char * first = value.s;
			const char * last = first + std::strlen(first);
			auto r = from_chars(first, last, version);
			BOOST_TEST_EQ(int(r.ec), int(std::errc {}));
			BOOST_TEST_EQ(value.x, version.at(0));
			BOOST_TEST_EQ(value.y, version.at(1));
			BOOST_TEST_EQ(value.z, version.at(2));
			BOOST_TEST_THROWS(version.at(3), std::out_of_range);
		}
	}
	{
		std::cout << "TEST: version_core<int, 2> from_chars\n";
		struct good_value
		{
			const char * s;
			int x;
			int y;
		};
		good_value good_values[] = {
			{ "0", 0, 0 },
			{ "0.0", 0, 0 },
			{ "1", 1, 0 },
			{ "1.1", 1, 1 },
			{ "1.0", 1, 0 },
			{ "9.9", 9, 9 },
			{ "99.99", 99, 99 },
			{ "999.999", 999, 999 },
			{ "9999.9999", 9999, 9999 },
			{ "99999.99999", 99999, 99999 },
			{ "999999.999999", 999999, 999999 },
			{ "9999999.9999999", 9999999, 9999999 },
			{ "99999999.99999999", 99999999, 99999999 },
			{ "13.72", 13, 72 },
		};
		for (auto & value : good_values)
		{
			std::cout << "> " << value.s << "\n";
			versioned::version_core<int, 2> version;
			const char * first = value.s;
			const char * last = first + std::strlen(first);
			auto r = from_chars(first, last, version);
			BOOST_TEST_EQ(int(r.ec), int(std::errc {}));
			BOOST_TEST_EQ(value.x, version.at(0));
			BOOST_TEST_EQ(value.y, version.at(1));
			BOOST_TEST_THROWS(version.at(3), std::out_of_range);
		}
	}
	{
		std::cout << "TEST: version_core<int, 3> to_string\n";
		struct good_value
		{
			const char * s;
			int x;
			int y;
			int z;
		};
		good_value good_values[] = {
			{ "0", 0, 0, 0 },
			{ "1", 1, 0, 0 },
			{ "1.1", 1, 1, 0 },
			{ "1.1.1", 1, 1, 1 },
			{ "9.9.9", 9, 9, 9 },
			{ "99.99.99", 99, 99, 99 },
			{ "999.999.999", 999, 999, 999 },
			{ "9999.9999.9999", 9999, 9999, 9999 },
			{ "99999.99999.99999", 99999, 99999, 99999 },
			{ "999999.999999.999999", 999999, 999999, 999999 },
			{ "9999999.9999999.9999999", 9999999, 9999999, 9999999 },
			{ "99999999.99999999.99999999", 99999999, 99999999, 99999999 },
			{ "13.72.6", 13, 72, 6 },
		};
		for (auto & value : good_values)
		{
			std::cout << "> " << value.s << "\n";
			versioned::version_core<int, 3> version(value.x, value.y, value.z);
			auto s = to_string(version);
			BOOST_TEST_CSTR_EQ(s.c_str(), value.s);
		}
	}
	{
		std::cout << "TEST: version_core<int, 3|2> compare\n";
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
		};
		for (auto & value : compare_values)
		{
			std::cout << "> " << value.a << " <=> " << value.b << "\n";
			{
				versioned::version_core<int, 3> a;
				versioned::version_core<int, 3> b;
				BOOST_TEST_EQ(
					int(from_chars(value.a, value.a + strlen(value.a), a).ec),
					int(std::errc {}));
				BOOST_TEST_EQ(
					int(from_chars(value.b, value.b + strlen(value.b), b).ec),
					int(std::errc {}));
				BOOST_TEST_NO_THROW(compare(a, b));
				BOOST_TEST_LT(compare(a, b), 0);
			}
			{
				versioned::version_core<int, 2> a;
				versioned::version_core<int, 3> b;
				BOOST_TEST_EQ(
					int(from_chars(value.a, value.a + strlen(value.a), a).ec),
					int(std::errc {}));
				BOOST_TEST_EQ(
					int(from_chars(value.b, value.b + strlen(value.b), b).ec),
					int(std::errc {}));
				BOOST_TEST_NO_THROW(compare(a, b));
				BOOST_TEST_LT(compare(a, b), 0);
			}
		}
	}
	{
		std::cout << "TEST: version_core<int> sort (op==, op<)\n";
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
		};
		std::vector<versioned::version_core<int>> base_vals;
		for (auto value : values)
		{
			versioned::version_core<int> a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			base_vals.push_back(a);
		}
		std::vector<versioned::version_core<int>> test_vals = base_vals;
		std::shuffle(test_vals.begin(), test_vals.end(),
			std::mt19937_64(732601397511647));
		std::sort(test_vals.begin(), test_vals.end());
		for (std::size_t i = 0; i < base_vals.size(); ++i)
		{
			std::cout << "> " << to_string(base_vals[i])
					  << " == " << to_string(test_vals[i]) << "\n";
			BOOST_TEST(base_vals[i] == test_vals[i]);
		}
	}
	{
		std::cout << "TEST: version_core<int> hash\n";
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
		};
		std::unordered_map<versioned::version_core<int>, const char *> m;
		for (auto value : values)
		{
			versioned::version_core<int> a;
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
			versioned::version_core<int> a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			BOOST_TEST_EQ(m[a], value);
		}
	}
	{
		std::cout << "TEST: version_core<int, 3> structured binding, get\n";
		struct test_value
		{
			const char * s;
			int x;
			int y;
			int z;
		};
		test_value values[] = {
			{ "0", 0, 0, 0 },
			{ "1", 1, 0, 0 },
			{ "1.1", 1, 1, 0 },
			{ "1.1.1", 1, 1, 1 },
			{ "9.9.9", 9, 9, 9 },
			{ "99.99.99", 99, 99, 99 },
			{ "999.999.999", 999, 999, 999 },
			{ "9999.9999.9999", 9999, 9999, 9999 },
			{ "99999.99999.99999", 99999, 99999, 99999 },
			{ "999999.999999.999999", 999999, 999999, 999999 },
			{ "9999999.9999999.9999999", 9999999, 9999999, 9999999 },
			{ "99999999.99999999.99999999", 99999999, 99999999, 99999999 },
			{ "13.72.6", 13, 72, 6 },
		};
		for (auto & value : values)
		{
			std::cout << "> " << value.s << "\n";
			versioned::version_core<int, 3> version(value.x, value.y, value.z);
			auto s = to_string(version);
			BOOST_TEST_EQ(::versioned::get<0>(version), value.x);
			BOOST_TEST_EQ(::versioned::get<1>(version), value.y);
			BOOST_TEST_EQ(::versioned::get<2>(version), value.z);
#if defined(__cpp_structured_bindings) && (__cpp_structured_bindings >= 201606L)
			auto [a, b, c] = version;
			BOOST_TEST_EQ(a, value.x);
			BOOST_TEST_EQ(b, value.y);
			BOOST_TEST_EQ(c, value.z);
#endif
		}
	}
	return boost::report_errors();
}
