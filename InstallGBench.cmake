#
#	CMake Install Google Benchmark by Parra Studios
#	CMake script to install Google Benchmark library.
#
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
		set(GBENCH_VERSION 1.4.1)
		set(GBENCH_URL_MD5 482dddb22bec43f5507a000456d6bb88)
	endif()

	ExternalProject_Add(GoogleBench
		DOWNLOAD_NAME	GBench-${GBENCH_VERSION}.tar.gz
		URL				https://github.com/google/benchmark/archive/v${GBENCH_VERSION}.tar.gz
		URL_MD5			${GBENCH_URL_MD5}
		CMAKE_ARGS		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON
		TEST_COMMAND	""
	)

	ExternalProject_Get_Property(GoogleBench INSTALL_DIR)

	if(MSVC)
		set(GBENCH_LIB_SUFFIX "lib")
	else()
		set(GBENCH_LIB_SUFFIX "a")
	endif()

	set(GBENCH_ROOT_DIR			"${INSTALL_DIR}")
	set(GBENCH_INCLUDE_DIR		"${GBENCH_ROOT_DIR}/include")
	set(GBENCH_LIBRARY			"${GBENCH_ROOT_DIR}/lib/libbenchmark.${GBENCH_LIB_SUFFIX}")
	set(GBENCH_FOUND			TRUE)

	mark_as_advanced(GBENCH_INCLUDE_DIR GBENCH_LIBRARY)

	message(STATUS "Install Google Benchmark v${GBENCH_VERSION}")
endif ()
