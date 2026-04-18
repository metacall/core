#
#	CMake Install Google Test by Parra Studios
#	CMake script to install Google Test library.
#
#	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

	if(MSVC AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
		if(OPTION_BUILD_THREAD_SANITIZER)
			set(SANITIZER_FLAGS -DCMAKE_CXX_FLAGS=/fsanitize=thread -DCMAKE_C_FLAGS=/fsanitize=thread)
		endif()
		if(OPTION_BUILD_ADDRESS_SANITIZER)
			set(SANITIZER_FLAGS -DCMAKE_CXX_FLAGS=/fsanitize=address -DCMAKE_C_FLAGS=/fsanitize=address)
		endif()
		if(OPTION_BUILD_MEMORY_SANITIZER)
			set(SANITIZER_FLAGS -DCMAKE_CXX_FLAGS="/fsanitize=memory /fsanitize=leak" -DCMAKE_C_FLAGS="/fsanitize=memory /fsanitize=leak")
		endif()
	endif()

	# Import Google Test Framework
	ExternalProject_Add(google-test-depends
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG v${GTEST_VERSION}
		PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
		CMAKE_ARGS
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			-Dgtest_build_samples=OFF
			-Dgtest_build_tests=OFF
			-Dgtest_disable_pthreads=${GTEST_DISABLE_PTHREADS}
			-Dgtest_force_shared_crt=ON
			-Dgtest_hide_internal_symbols=OFF
			-DINSTALL_GTEST=ON
			-DBUILD_GMOCK=ON
			-Dgmock_build_tests=OFF
			${SANITIZER_FLAGS}
		UPDATE_COMMAND ""
		TEST_COMMAND ""
	)

	# Google Test include and binary directories
	ExternalProject_Get_Property(google-test-depends install_dir)

	execute_process(
		COMMAND ${CMAKE_COMMAND} -E echo "${install_dir}"
	)

	execute_process(
		COMMAND ls -laR "${install_dir}"
	)

	# Create imported targets
	add_library(gtest STATIC IMPORTED)
	add_library(gmock STATIC IMPORTED)

	set_target_properties(gtest PROPERTIES
		IMPORTED_LOCATION "${install_dir}/lib/libgtest.a"
		INTERFACE_INCLUDE_DIRECTORIES "${install_dir}/include"
	)

	set_target_properties(gmock PROPERTIES
		IMPORTED_LOCATION "${install_dir}/lib/libgmock.a"
		INTERFACE_INCLUDE_DIRECTORIES "${install_dir}/include"
	)

	# Ensure build order
	add_dependencies(gtest google-test-depends)
	add_dependencies(gmock google-test-depends)

	set(GTEST_LIBRARY gtest)
	set(GMOCK_LIBRARY gmock)
	set(GTEST_LIBRARIES gtest gmock Threads::Threads)
	set(GTEST_INCLUDE_DIRS "${install_dir}/include")
	set(GTEST_FOUND TRUE)

	mark_as_advanced(
		GTEST_LIBRARY
		GMOCK_LIBRARY
		GTEST_LIBRARIES
		GTEST_INCLUDE_DIRS
	)

	message(STATUS "Install Google Test v${GTEST_VERSION}")
endif()
