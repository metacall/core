#
#	CMake Find Google Benchmark by Parra Studios
#	CMake script to find Google Benchmark library.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Prevent vervosity if already included
if(GBENCH_FOUND)
	set(GBENCH_FIND_QUIETLY TRUE)
endif()

find_path(GBENCH_INCLUDE_DIR NAMES benchmark/benchmark.h)
find_library(GBENCH_LIBRARY NAMES benchmark)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(google-benchmark
	REQUIRED_VARS GBENCH_INCLUDE_DIR GBENCH_LIBRARY
)

mark_as_advanced(GBENCH_INCLUDE_DIR GBENCH_LIBRARY)

if(GBENCH_FOUND)
	if(NOT GBENCH_FIND_QUIETLY)
		message(STATUS "Found Google Benchmark:\n\tLibrary: ${GBENCH_LIBRARY}\n\tIncludes: ${GBENCH_INCLUDE_PATH}")
	endif(NOT GBENCH_FIND_QUIETLY)
else(GBENCH_FOUND)
	if(GBENCH_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find Google Benchmark library")
	endif(GBENCH_FIND_REQUIRED)
endif(GBENCH_FOUND)
