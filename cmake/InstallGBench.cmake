#
#	CMake Install Google Benchmark by Parra Studios
#	CMake script to install Google Benchmark library.
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
# GBENCH_INCLUDE_DIR - Path where Google Benchmark header is located.
# GBENCH_LIBRARY - Path where Google Benchmark library is located.

if(NOT GBENCH_FOUND OR USE_BUNDLED_GBENCH)
	if(NOT GBENCH_VERSION OR USE_BUNDLED_GBENCH)
		set(GBENCH_VERSION 1.6.1)
	endif()

	include(FetchContent)

	# Import Google Benchmark
	FetchContent_Declare(
		googlebenchmark
		GIT_REPOSITORY https://github.com/google/benchmark.git
		GIT_TAG v${GBENCH_VERSION}
	)

	# Benchmark options
	set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "" FORCE)
	set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

	if(MSVC)
		# Build statically on Windows for avoiding DLL location issues (avoid populating the variable to the cache)
		set(_CACHE_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
		set(BUILD_SHARED_LIBS OFF)
	endif()

	FetchContent_MakeAvailable(googlebenchmark)

	if(MSVC)
		# Restore shared library value
		set(BUILD_SHARED_LIBS ${_CACHE_BUILD_SHARED_LIBS})
	endif()

	set(GBENCH_ROOT_DIR
		${googlebenchmark_SOURCE_DIR}
	)

	set(GBENCH_INCLUDE_DIR
		${googlebenchmark_SOURCE_DIR}/include
	)

	set(GBENCH_LIBRARY
		benchmark
	)

	set(GBENCH_FOUND TRUE)

	mark_as_advanced(
		GBENCH_INCLUDE_DIR
		GBENCH_LIBRARY
	)

	message(STATUS "Fetch Google Benchmark v${GBENCH_VERSION}")
endif()
