#
#	CMake Install Google Test by Parra Studios
#	CMake script to install Google Test library.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

# The following variables are set:
#
# GTEST_INCLUDE_DIRS - A list of directories where the Google Test headers are located.
# GTEST_LIBRARIES - A list of directories where the Google Test libraries are located.

if(NOT GTEST_FOUND OR USE_BUNDLED_GTEST)
	if(NOT GTEST_VERSION OR USE_BUNDLED_GTEST)
		set(GTEST_VERSION 1.8.1)
	endif()

	find_package(Threads REQUIRED)

	if(MINGW)
		set(GTEST_DISABLE_PTHREADS ON)
	else()
		set(GTEST_DISABLE_PTHREADS OFF)
	endif()

	# Import Google Test Framework
	ExternalProject_Add(google-test-depends
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG release-${GTEST_VERSION}
		CMAKE_ARGS -Dgmock_build_tests=OFF
			-Dgtest_build_samples=OFF
			-Dgtest_build_tests=OFF
			-Dgtest_disable_pthreads=${GTEST_DISABLE_PTHREADS}
			-Dgtest_force_shared_crt=ON
			-Dgtest_hide_internal_symbols=OFF
			-DINSTALL_GTEST=OFF
			-DBUILD_GMOCK=ON
		PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
		UPDATE_COMMAND ""
		INSTALL_COMMAND ""
		TEST_COMMAND ""
	)

	# Google Test include and binary directories
	ExternalProject_Get_Property(google-test-depends source_dir binary_dir)

	set(GTEST_INCLUDE_DIR "${source_dir}/googletest/include")
	set(GMOCK_INCLUDE_DIR "${source_dir}/googlemock/include")

	if(MSVC)
		set(GTEST_LIB_PREFIX "")
		set(GTEST_LIB_SUFFIX "lib")
		set(GTEST_LIBS_DIR "${binary_dir}/googlemock/gtest/${CMAKE_BUILD_TYPE}")
		set(GMOCK_LIBS_DIR "${binary_dir}/googlemock/${CMAKE_BUILD_TYPE}")
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			set(GTEST_LIB_DEBUG "d")
		else()
			set(GTEST_LIB_DEBUG "")
		endif()
	else()
		set(GTEST_LIB_PREFIX "lib")
		set(GTEST_LIB_SUFFIX "a")
		set(GTEST_LIBS_DIR "${binary_dir}/googlemock/gtest")
		set(GMOCK_LIBS_DIR "${binary_dir}/googlemock")
		set(GTEST_LIB_DEBUG "")
	endif()

	# Define Paths
	set(GTEST_INCLUDE_DIRS
		"${GTEST_INCLUDE_DIR}"
		"${GMOCK_INCLUDE_DIR}"
	)

	set(GTEST_LIBRARIES
		"${GTEST_LIBS_DIR}/${GTEST_LIB_PREFIX}gtest${GTEST_LIB_DEBUG}.${GTEST_LIB_SUFFIX}"
		"${GMOCK_LIBS_DIR}/${GTEST_LIB_PREFIX}gmock${GTEST_LIB_DEBUG}.${GTEST_LIB_SUFFIX}"
		"${CMAKE_THREAD_LIBS_INIT}"
	)

	set(GTEST_FOUND TRUE)

	mark_as_advanced(GTEST_INCLUDE_DIRS GTEST_LIBRARIES)

	message(STATUS "Install Google Test v${GTEST_VERSION}")
endif ()
