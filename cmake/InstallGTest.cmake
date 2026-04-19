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

	# # TODO:
	# if(MSVC AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
	# 	if(OPTION_BUILD_THREAD_SANITIZER)
	# 		set(SANITIZER_FLAGS -DCMAKE_CXX_FLAGS=/fsanitize=thread -DCMAKE_C_FLAGS=/fsanitize=thread)
	# 	endif()
	# 	if(OPTION_BUILD_ADDRESS_SANITIZER)
	# 		set(SANITIZER_FLAGS -DCMAKE_CXX_FLAGS=/fsanitize=address -DCMAKE_C_FLAGS=/fsanitize=address)
	# 	endif()
	# 	if(OPTION_BUILD_MEMORY_SANITIZER)
	# 		set(SANITIZER_FLAGS -DCMAKE_CXX_FLAGS="/fsanitize=memory /fsanitize=leak" -DCMAKE_C_FLAGS="/fsanitize=memory /fsanitize=leak")
	# 	endif()
	# endif()

	include(FetchContent)

	FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/refs/tags/v${GTEST_VERSION}.zip
	)

	# Setup
	set(gtest_build_samples OFF CACHE BOOL "" FORCE)
	set(gtest_build_tests OFF CACHE BOOL "" FORCE)
	set(gtest_disable_pthreads ${GTEST_DISABLE_PTHREADS} CACHE BOOL "" FORCE)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	set(gtest_hide_internal_symbols OFF CACHE BOOL "" FORCE)
	set(gmock_build_tests OFF CACHE BOOL "" FORCE)

	FetchContent_MakeAvailable(googletest)

	# Include dirs
	set(GTEST_INCLUDE_DIRS
		${gtest_SOURCE_DIR}/googletest/include
		${gmock_SOURCE_DIR}/googlemock/include
	)

	# Librarie
	set(GTEST_LIBRARY GTest::gtest)
	set(GMOCK_LIBRARY GTest::gmock)
	set(GTEST_LIBRARIES
		GTEST_LIBRARY
		GMOCK_LIBRARY
	)
	set(GTEST_FOUND TRUE)

	mark_as_advanced(
		GTEST_LIBRARY
		GMOCK_LIBRARY
		GTEST_LIBRARIES
		GTEST_INCLUDE_DIRS
	)

	message(STATUS "Install Google Test v${GTEST_VERSION}")
endif()
