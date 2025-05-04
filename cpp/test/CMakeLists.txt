# Copyright René Ferdinand Rivera Morell 2025
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required( VERSION 3.31 )

file( GLOB RUN_TESTS_SRCS *_run_test.cpp )

foreach( TEST_SRC ${RUN_TESTS_SRCS} )
	get_filename_component( TEST ${TEST_SRC} NAME_WLE )
	add_executable( ${TEST} ${TEST_SRC} )
	target_link_libraries( ${TEST} bfg_versioned )
	target_include_directories( ${TEST} PRIVATE ext_boost_core/include )
	add_test( NAME ${TEST} COMMAND ${TEST} )
endforeach()
