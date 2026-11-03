#
#	CMake Install SimdJSON by Parra Studios
#	CMake script to install SimdJSON library.
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
# SIMDJSON_LIBRARIES - The SimdJSON static library target (compiled with -fPIC).
# SIMDJSON_COMPILE_DEFINITIONS - Compile definitions shared by the library build and its consumers.
# SIMDJSON_CXX_FLAGS - Custom SimdJSON compilation flags.

if(NOT SIMDJSON_FOUND OR USE_BUNDLED_SIMDJSON)
	if(NOT SIMDJSON_VERSION OR USE_BUNDLED_SIMDJSON)
		set(SIMDJSON_VERSION 5.0.5)
	endif()

	include(SimdJSONOptions)

	include(FetchContent)

	# SimdJSON is built as part of the project, so it inherits the toolchain (i.e: Android), build type and sanitizers
	FetchContent_Declare(
		simdjson
		GIT_REPOSITORY https://github.com/simdjson/simdjson.git
		GIT_TAG v${SIMDJSON_VERSION}
		GIT_SHALLOW TRUE
	)

	if(SIMDJSON_UTF8VALIDATION)
		set(SIMDJSON_SKIPUTF8VALIDATION OFF CACHE BOOL "" FORCE)
	else()
		set(SIMDJSON_SKIPUTF8VALIDATION ON CACHE BOOL "" FORCE)
	endif()

	set(SIMDJSON_DEVELOPER_MODE OFF CACHE BOOL "" FORCE)
	set(SIMDJSON_ENABLE_THREADS OFF CACHE BOOL "" FORCE)
	set(SIMDJSON_INSTALL OFF CACHE BOOL "" FORCE)

	# Build statically, it is linked into the serial library (avoid populating the variable to the cache)
	set(_CACHE_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
	set(BUILD_SHARED_LIBS OFF)

	FetchContent_MakeAvailable(simdjson)

	# Restore shared library value
	set(BUILD_SHARED_LIBS ${_CACHE_BUILD_SHARED_LIBS})

	# SimdJSON is linked into shared libraries, so compile it as PIC
	set_target_properties(simdjson PROPERTIES
		POSITION_INDEPENDENT_CODE ON
	)

	# Build the library with the same definitions used by the consumers
	foreach(definition IN LISTS SIMDJSON_COMPILE_DEFINITIONS)
		# UTF-8 validation is always defined by SimdJSON build system through SIMDJSON_SKIPUTF8VALIDATION
		if(NOT definition MATCHES "^SIMDJSON_UTF8VALIDATION=")
			target_compile_definitions(simdjson PRIVATE ${definition})
		endif()
	endforeach()

	separate_arguments(SIMDJSON_CXX_OPTIONS NATIVE_COMMAND "${SIMDJSON_CXX_FLAGS}")

	target_compile_options(simdjson PRIVATE ${SIMDJSON_CXX_OPTIONS})

	set(SIMDJSON_INCLUDE_DIRS	"${simdjson_SOURCE_DIR}/include")
	set(SIMDJSON_LIBRARIES		simdjson::simdjson)

	set(SIMDJSON_FOUND TRUE)

	mark_as_advanced(SIMDJSON_INCLUDE_DIRS SIMDJSON_LIBRARIES)

	message(STATUS "Installing SimdJSON v${SIMDJSON_VERSION}")
endif()
