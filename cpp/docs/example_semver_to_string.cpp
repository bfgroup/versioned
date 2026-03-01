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
	semver sv;
	sv.version().at(0) = 3;
	sv.version().at(1) = 1;
	sv.version().at(2) = 4;
	sv.prerelease() = semver<>::prerelease_t { "alpha", "2" };
	sv.build() = semver<>::build_t { "PI" };
	std::cout << to_string(sv) << "\n";
}

// end::example-code[]
