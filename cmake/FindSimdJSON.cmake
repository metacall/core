#
#	CMake Find SimdJSON by Parra Studios
#	CMake script to find SimdJSON library.
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
# SIMDJSON_INCLUDE_DIRS - A list of directories where the SimdJSON headers are located.
# SIMDJSON_LIBRARIES - The SimdJSON static library (compiled with -fPIC).
# SIMDJSON_COMPILE_DEFINITIONS - Compile definitions shared by the library build and its consumers.
# SIMDJSON_CXX_FLAGS - Custom SimdJSON compilation flags.
#
# The following options can be set (as CMake cache entries or environment variables):
#
# SIMDJSON_INCLUDE_DIR - Additional path where to look for the SimdJSON headers.
# SIMDJSON_LIBRARY_DIR - Additional path where to look for the SimdJSON library.
# SIMDJSON_UTF8VALIDATION - Validate UTF-8 on parsing (OFF by default, like RapidJSON).
# SIMDJSON_IMPLEMENTATION - Semicolon separated list of implementations to include
#	(fallback, westmere, haswell, icelake, arm64, ppc64), empty includes all supported ones.
# SIMDJSON_EXTRA_CXX_FLAGS - Extra compilation flags (i.e: -mno-avx -mno-avx2).
# SIMDJSON_EXTRA_DEFINITIONS - Extra compile definitions (i.e: SIMDJSON_EXPERIMENTAL_HAS_SSE2=0).

# Prevent vervosity if already included
if(SIMDJSON_FOUND)
	set(SIMDJSON_FIND_QUIETLY TRUE)
endif()

foreach(opt SIMDJSON_INCLUDE_DIR SIMDJSON_LIBRARY_DIR SIMDJSON_UTF8VALIDATION SIMDJSON_IMPLEMENTATION SIMDJSON_EXTRA_CXX_FLAGS SIMDJSON_EXTRA_DEFINITIONS)
	if(${opt} AND DEFINED ENV{${opt}} AND NOT ${opt} STREQUAL "$ENV{${opt}}")
		message(WARNING "Conflicting ${opt} values: ignoring environment variable and using CMake cache entry")
	elseif(DEFINED ENV{${opt}} AND NOT ${opt})
		set(${opt} "$ENV{${opt}}")
	endif()
endforeach()

# Compile definitions must be the same for the library and the code including its headers
include(SimdJSONOptions)

# Default SimdJSON paths
set(SIMDJSON_INCLUDE_DIR
	"${SIMDJSON_INCLUDE_DIR}"
	"/usr/local/include/"
)

set(SIMDJSON_LIBRARY_DIR
	"${SIMDJSON_LIBRARY_DIR}"
	"/usr/local/lib/"
)

find_path(
	SIMDJSON_INCLUDE_DIRS
	NAMES simdjson.h
	PATHS ${SIMDJSON_INCLUDE_DIR}
	DOC "Include directory for the SimdJSON library"
)

# Only the static version is supported, it must be compiled with -fPIC
find_library(
	SIMDJSON_LIBRARIES
	NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}simdjson${CMAKE_STATIC_LIBRARY_SUFFIX}
	PATHS ${SIMDJSON_LIBRARY_DIR}
	DOC "Static library of SimdJSON"
)

mark_as_advanced(SIMDJSON_INCLUDE_DIRS SIMDJSON_LIBRARIES)

if(SIMDJSON_INCLUDE_DIRS AND SIMDJSON_LIBRARIES)
	set(SIMDJSON_FOUND TRUE)
endif()

mark_as_advanced(SIMDJSON_FOUND)

if(SIMDJSON_FOUND)
	if(NOT SIMDJSON_FIND_QUIETLY)
		message(STATUS "Found SimdJSON header files in ${SIMDJSON_INCLUDE_DIRS}")
		message(STATUS "Found SimdJSON library ${SIMDJSON_LIBRARIES}")
		message(STATUS "Found SimdJSON compile definitions: ${SIMDJSON_COMPILE_DEFINITIONS}")
		if(SIMDJSON_CXX_FLAGS)
			message(STATUS "Found SimdJSON C++ extra compilation flags: ${SIMDJSON_CXX_FLAGS}")
		endif()
	endif()
elseif(SIMDJSON_FIND_REQUIRED)
	message(FATAL_ERROR "Could not find SimdJSON")
endif()
