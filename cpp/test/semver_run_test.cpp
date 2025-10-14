// Copyright René Ferdinand Rivera Morell
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <cstdint>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

#include <versioned/versioned.hpp>

#include <boost/core/lightweight_test.hpp>

int main()
{
	BOOST_TEST(true);
	{
		std::cout << "TEST: semver<> from_chars\n";
		struct T
		{
			const char * full;
			int major;
			int minor;
			int patch;
			const char * pre;
			const char * build;
		};
		T data[] = {
			{ "0", 0, 0, 0, "", "" },
			{ "0.0", 0, 0, 0, "", "" },
			{ "0.0.0", 0, 0, 0, "", "" },
			{ "1", 1, 0, 0, "", "" },
			{ "1.1-alpha.1", 1, 1, 0, "alpha.1", "" },
			{ "1.1.1", 1, 1, 1, "", "" },
			{ "1.0.0", 1, 0, 0, "", "" },
			{ "9.9.9-0.3.7", 9, 9, 9, "0.3.7", "" },
			{ "99.99.99-alpha+21AF26D3----117B344092BD", 99, 99, 99, "alpha",
				"21AF26D3----117B344092BD" },
			{ "999.999.999", 999, 999, 999, "", "" },
			{ "9999.9999.9999+20130313144700", 9999, 9999, 9999, "",
				"20130313144700" },
			{ "99999.99999.99999", 99999, 99999, 99999, "", "" },
			{ "999999.999999.999999", 999999, 999999, 999999, "", "" },
			{ "9999999.9999999.9999999", 9999999, 9999999, 9999999, "", "" },
			{ "99999999.99999999.99999999", 99999999, 99999999, 99999999, "",
				"" },
			{ "13.72.6", 13, 72, 6, "", "" },
		};
		for (auto & d : data)
		{
			std::cout << "> " << d.full << "\n";
			versioned::semver<> ver;
			const char * first = d.full;
			const char * last = first + std::strlen(first);
			auto r = from_chars(first, last, ver);
			BOOST_TEST_EQ(int(r.ec), int(std::errc {}));
			BOOST_TEST_EQ(ver.major(), d.major);
			BOOST_TEST_EQ(ver.minor(), d.minor);
			BOOST_TEST_EQ(ver.patch(), d.patch);
			BOOST_TEST_EQ(to_string(ver.prerelease()), d.pre);
			BOOST_TEST_EQ(to_string(ver.build()), d.build);
			if (!ver.prerelease().empty() || !ver.build().empty())
			{
				std::string s = to_string(ver);
				BOOST_TEST_EQ(s, d.full);
			}
		}
	}
	{
		std::cout << "TEST: semver<> compare\n";
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
			{ "1.0.0-alpha", "1.0.0-alpha.1" },
			{ "1.0.0-alpha.1", "1.0.0-alpha.beta" },
			{ "1.0.0-alpha.beta", "1.0.0-beta" },
			{ "1.0.0-beta", "1.0.0-beta.2" },
			{ "1.0.0-beta.2", "1.0.0-beta.11" },
			{ "1.0.0-beta.11", "1.0.0-rc.1" },
			{ "1.0.0-rc.1", "1.0.0" },
		};
		for (auto & value : compare_values)
		{
			std::cout << "> " << value.a << " <=> " << value.b << "\n";
			{
				versioned::semver<> a;
				versioned::semver<> b;
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
		std::cout << "TEST: semver<> sort (op==, op<)\n";
		const char * values[] = {
			"1.0.0-alpha",
			"1.0.0-alpha.1",
			"1.0.0-alpha.beta",
			"1.0.0-beta",
			"1.0.0-beta.2",
			"1.0.0-beta.11",
			"1.0.0-rc.1",
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
		std::vector<versioned::semver<>> base_vals;
		for (auto value : values)
		{
			versioned::semver<> a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			base_vals.push_back(a);
		}
		std::vector<versioned::semver<>> test_vals = base_vals;
		std::shuffle(test_vals.begin(), test_vals.end(),
			std::mt19937_64(291020249555917));
		std::sort(test_vals.begin(), test_vals.end());
		for (std::size_t i = 0; i < base_vals.size(); ++i)
		{
			std::cout << "> " << to_string(base_vals[i])
					  << " == " << to_string(test_vals[i]) << "\n";
			BOOST_TEST(base_vals[i] == test_vals[i]);
		}
	}
	{
		std::cout << "TEST: semver<> hash\n";
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
			"1.0.0-alpha",
			"1.0.0-alpha.1",
			"1.0.0-alpha.beta",
			"1.0.0-beta",
			"1.0.0-beta.2",
			"1.0.0-beta.11",
			"1.0.0-rc.1",
		};
		std::unordered_map<versioned::semver<>, const char *> m;
		for (auto value : values)
		{
			versioned::semver<> a;
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
			versioned::semver<> a;
			BOOST_TEST_EQ(int(from_chars(value, value + strlen(value), a).ec),
				int(std::errc {}));
			BOOST_TEST_EQ(m[a], value);
		}
	}
	{
		std::cout << "TEST: semver<> structured binding\n";
		struct T
		{
			const char * full;
			int major;
			int minor;
			int patch;
			const char * pre;
			const char * build;
		};
		T data[] = {
			{ "0", 0, 0, 0, "", "" },
			{ "0.0", 0, 0, 0, "", "" },
			{ "0.0.0", 0, 0, 0, "", "" },
			{ "1", 1, 0, 0, "", "" },
			{ "1.1-alpha.1", 1, 1, 0, "alpha.1", "" },
			{ "1.1.1", 1, 1, 1, "", "" },
			{ "1.0.0", 1, 0, 0, "", "" },
			{ "9.9.9-0.3.7", 9, 9, 9, "0.3.7", "" },
			{ "99.99.99-alpha+21AF26D3----117B344092BD", 99, 99, 99, "alpha",
				"21AF26D3----117B344092BD" },
			{ "999.999.999", 999, 999, 999, "", "" },
			{ "9999.9999.9999+20130313144700", 9999, 9999, 9999, "",
				"20130313144700" },
			{ "99999.99999.99999", 99999, 99999, 99999, "", "" },
			{ "999999.999999.999999", 999999, 999999, 999999, "", "" },
			{ "9999999.9999999.9999999", 9999999, 9999999, 9999999, "", "" },
			{ "99999999.99999999.99999999", 99999999, 99999999, 99999999, "",
				"" },
			{ "13.72.6", 13, 72, 6, "", "" },
		};
		for (auto & d : data)
		{
			std::cout << "> " << d.full << "\n";
			versioned::semver<> ver;
			const char * first = d.full;
			const char * last = first + std::strlen(first);
			auto r = from_chars(first, last, ver);
			BOOST_TEST_EQ(int(r.ec), int(std::errc {}));
#if defined(__cpp_structured_bindings) && (__cpp_structured_bindings >= 201606L)
			auto [v, p, b] = ver;
			auto [x, y, z] = v;
			BOOST_TEST_EQ(x, d.major);
			BOOST_TEST_EQ(y, d.minor);
			BOOST_TEST_EQ(z, d.patch);
			BOOST_TEST_EQ(to_string(p), d.pre);
			BOOST_TEST_EQ(to_string(b), d.build);
#endif
		}
	}
	return boost::report_errors();
}
