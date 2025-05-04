# Copyright René Ferdinand Rivera Morell 2025
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required( VERSION 3.31 )

project( bfg_versioned
	VERSION "0.1"
	DESCRIPTION ""
	HOMEPAGE_URL "https://github.com/bfgroup/versioned"
	LANGUAGES CXX )

add_library( bfg_versioned INTERFACE )
add_library( BFG::versioned ALIAS bfg_versioned )
target_include_directories( bfg_versioned INTERFACE include )
target_compile_features(bfg_versioned INTERFACE cxx_std_11)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  include(CTest) # defines BUILD_TESTING
endif()

if( BUILD_TESTING )
  add_subdirectory(test)
endif()
