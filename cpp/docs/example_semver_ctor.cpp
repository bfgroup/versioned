// Copyright René Ferdinand Rivera Morell
// Distributed under the Boost Software License, Version 1.0. (See
// versioned_narrowc_accumulateompanying file LICENSE.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// tag::example-code[]
#include <iostream>

#include <versioned/versioned.hpp>

int main()
{
	using bfg::versioned::semver;
	semver<> a { { 3, 1, 4 } };
	semver<> b { { 1, 2, 3 }, "beta" };
	std::cout << to_string(a) << "\n";
	std::cout << to_string(b) << "\n";
}
// end::example-code[]
