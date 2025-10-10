// Copyright René Ferdinand Rivera Morell
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <typeinfo>
#include <vector>

#include <versioned/versioned.hpp>

template <typename Int>
struct test
{
	void run()
	{
		parse_f parse_functions[] = {
			{ "std::from_chars", &test::std_from_chars_10 },
			{ "std::strtox    ", &test::std_strtox_10 },
			{ "versioned      ", &versioned::detail::from_chars_10<Int> },
		};
		static const auto value_count = 5000000;
		// Generate numbers to parse.
		std::string values_str;
		{
			// std::random_device r;
			// std::mt19937 g(r());
			std::mt19937 g(349167852);
			std::uniform_int_distribution<> d(
				0, std::numeric_limits<Int>::max());
			values_str.reserve(value_count * 5);
			for (size_t i = 0; i < value_count; ++i)
			{
				auto value = d(g);
				if (i != 0) values_str.append(1, '.');
				values_str.append(std::to_string(value));
			}
		}
		// Run the benchmark for each method.
		double base_million_per_second = 0;
		for (auto f : parse_functions)
		{
			// Run the function twice to warm up the caches.
			perf_for_function(f.function, values_str, &test_int);
			perf_for_function(f.function, values_str, &test_int);
			// Run the function 3, or more, times to record timing.
			std::array<std::chrono::steady_clock::duration, 5> samples;
			for (auto & sample : samples)
			{
				auto t0 = std::chrono::steady_clock::now();
				perf_for_function(f.function, values_str, &test_int);
				auto t1 = std::chrono::steady_clock::now();
				sample = t1 - t0;
			}
			// Average the samples and report that as the time.
			std::sort(samples.begin(), samples.end());
			double million_per_second = 0;
			for (auto & sample : samples)
				million_per_second += (value_count
					/ std::chrono::duration<double>(sample).count()
					/ 1000000.0);
			million_per_second /= double(samples.size());
			std::cout << f.name << " : " << million_per_second << " M"
					  << typeid(Int).name() << "/s";
			if (base_million_per_second == 0)
				base_million_per_second = million_per_second;
			else
				std::cout << " ("
						  << (million_per_second / base_million_per_second)
						  << ")";
			std::cout << "\n";
		}
	}

	private:
	using from_chars_f = const char * (*)(const char *, const char *, Int &);
	struct parse_f
	{
		const char * name = nullptr;
		from_chars_f function = nullptr;
	};

	static void perf_for_function(
		from_chars_f f, const std::string & v, Int * r)
	{
		const char * v_first = v.c_str();
		const char * v_last = v_first + v.size();
		while (v_first < v_last)
		{
			v_first = f(v_first, v_last, *r) + 1;
			value_eval(r);
		}
	}

	static void value_eval(Int * i);

	Int test_int {};

	static const char * std_from_chars_10(
		const char * first, const char * last, Int & value)
	{
		auto r = std::from_chars(first, last, value, 10);
		return r.ec == std::errc {} ? r.ptr : last;
	}

	static const char * std_strtox_10(
		const char * first, const char * last, Int & value);
};

template <>
const char * test<int>::std_strtox_10(
	const char * first, const char * last, int & value)
{
	char * result = const_cast<char *>(last);
	value = static_cast<int>(std::strtol(first, &result, 10));
	return result;
}

int main()
{
	test<int> test_int;
	test_int.run();
}

template <>
void test<int>::value_eval(int * value)
{}
