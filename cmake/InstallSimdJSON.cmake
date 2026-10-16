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
# SIMDJSON_LIBRARIES - The SimdJSON static library (compiled with -fPIC).
# SIMDJSON_COMPILE_DEFINITIONS - Compile definitions shared by the library build and its consumers.
# SIMDJSON_CXX_FLAGS - Custom SimdJSON compilation flags.

if(NOT SIMDJSON_FOUND OR USE_BUNDLED_SIMDJSON)
	if(NOT SIMDJSON_VERSION OR USE_BUNDLED_SIMDJSON)
		set(SIMDJSON_VERSION 5.0.5)
	endif()

	include(SimdJSONOptions)

	# Build the compile flags of the library with the same definitions used by the consumers
	set(SIMDJSON_BUILD_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SIMDJSON_CXX_FLAGS}")

	foreach(definition IN LISTS SIMDJSON_COMPILE_DEFINITIONS)
		# UTF-8 validation is always defined by SimdJSON build system through SIMDJSON_SKIPUTF8VALIDATION
		if(NOT definition MATCHES "^SIMDJSON_UTF8VALIDATION=")
			set(SIMDJSON_BUILD_CXX_FLAGS "${SIMDJSON_BUILD_CXX_FLAGS} -D${definition}")
		endif()
	endforeach()

	if(SIMDJSON_UTF8VALIDATION)
		set(SIMDJSON_SKIPUTF8VALIDATION OFF)
	else()
		set(SIMDJSON_SKIPUTF8VALIDATION ON)
	endif()

	set(SIMDJSON_ROOT_DIR		"${CMAKE_CURRENT_BINARY_DIR}/simdjson")
	set(SIMDJSON_INCLUDE_DIRS	"${SIMDJSON_ROOT_DIR}/include")
	set(SIMDJSON_LIBRARIES		"${SIMDJSON_ROOT_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}simdjson${CMAKE_STATIC_LIBRARY_SUFFIX}")

	ExternalProject_Add(simd-json-depends
		GIT_REPOSITORY		"https://github.com/simdjson/simdjson.git"
		GIT_TAG				"v${SIMDJSON_VERSION}"
		GIT_SHALLOW			TRUE
		CMAKE_ARGS
			-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
			-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} # Use the same build type as MetaCall
			-DCMAKE_CXX_FLAGS=${SIMDJSON_BUILD_CXX_FLAGS}
			-DCMAKE_POSITION_INDEPENDENT_CODE=ON
			-DCMAKE_INSTALL_PREFIX=${SIMDJSON_ROOT_DIR}
			-DCMAKE_INSTALL_LIBDIR=lib
			-DBUILD_SHARED_LIBS=OFF
			-DSIMDJSON_DEVELOPER_MODE=OFF
			-DSIMDJSON_SINGLEHEADER=ON # Install the single header
			-DSIMDJSON_SKIPUTF8VALIDATION=${SIMDJSON_SKIPUTF8VALIDATION}
			-DSIMDJSON_ENABLE_THREADS=OFF
			-DSIMDJSON_INSTALL=ON
		BUILD_BYPRODUCTS	"${SIMDJSON_LIBRARIES}"
		TEST_COMMAND		""
	)

	set(SIMDJSON_FOUND TRUE)

	mark_as_advanced(SIMDJSON_INCLUDE_DIRS SIMDJSON_LIBRARIES)

	message(STATUS "Installing SimdJSON v${SIMDJSON_VERSION}")
endif()
