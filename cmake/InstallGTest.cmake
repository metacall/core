#
#	CMake Install Google Test by Parra Studios
#	CMake script to install Google Test library.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
		set(GTEST_VERSION 1.16.0)
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
		GIT_TAG v${GTEST_VERSION}
		CMAKE_ARGS
			-Dgtest_build_samples=OFF
			-Dgtest_build_tests=OFF
			-Dgtest_disable_pthreads=${GTEST_DISABLE_PTHREADS}
			-Dgtest_force_shared_crt=ON
			-Dgtest_hide_internal_symbols=OFF
			-DINSTALL_GTEST=OFF
			-DBUILD_GMOCK=ON
			-Dgmock_build_tests=OFF
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
		set(GTEST_LIBS_DIR "${binary_dir}/lib/${CMAKE_BUILD_TYPE}")
		set(GMOCK_LIBS_DIR "${binary_dir}/lib/${CMAKE_BUILD_TYPE}")
	else()
		set(GTEST_LIB_PREFIX "lib")
		set(GTEST_LIB_SUFFIX "a")
		set(GTEST_LIBS_DIR "${binary_dir}/lib")
		set(GMOCK_LIBS_DIR "${binary_dir}/lib")
	endif()

	# Define Paths
	set(GTEST_INCLUDE_DIRS
		"${GTEST_INCLUDE_DIR}"
		"${GMOCK_INCLUDE_DIR}"
	)

	set(GTEST_LIBRARY
		"${GTEST_LIBS_DIR}/${GTEST_LIB_PREFIX}gtest.${GTEST_LIB_SUFFIX}"
	)

	set(GMOCK_LIBRARY
		"${GMOCK_LIBS_DIR}/${GTEST_LIB_PREFIX}gmock.${GTEST_LIB_SUFFIX}"
	)

	set(GTEST_LIBRARIES
		"${GTEST_LIBRARY}"
		"${GMOCK_LIBRARY}"
		"${CMAKE_THREAD_LIBS_INIT}"
	)

	set(GTEST_FOUND TRUE)

	mark_as_advanced(
		GTEST_LIBRARY
		GMOCK_LIBRARY
		GTEST_LIBRARIES
		GTEST_INCLUDE_DIR
		GMOCK_INCLUDE_DIR
		GTEST_INCLUDE_DIRS
	)

	message(STATUS "Install Google Test v${GTEST_VERSION}")
endif()
