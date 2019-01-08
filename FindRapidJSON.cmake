#
#	CMake Find RapidJSON by Parra Studios
#	CMake script to find RapidJSON library.
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
# RAPIDJSON_CXX_FLAGS - Custom RapidJSON compilation flags.
# RAPIDJSON_INCLUDE_DIRS - A list of directories where the RapidJSON headers are located.

# Prevent vervosity if already included
if(RAPIDJSON_FOUND)
	set(RAPIDJSON_FIND_QUIETLY TRUE)
endif()

foreach(opt RAPIDJSON_INCLUDE_DIR RAPIDJSON_USE_SSE2 RAPIDJSON_USE_SSE42)
	if(${opt} AND DEFINED ENV{${opt}} AND NOT ${opt} STREQUAL "$ENV{${opt}}")
		message(WARNING "Conflicting ${opt} values: ignoring environment variable and using CMake cache entry")
	elseif(DEFINED ENV{${opt}} AND NOT ${opt})
		set(${opt} "$ENV{${opt}}")
	endif()
endforeach()

# Default RapidJSON include paths
set(RAPIDJSON_INCLUDE_DIR
	"${RAPIDJSON_INCLUDE_DIR}"
	"/usr/local/include/"
)

find_path(
	RAPIDJSON_INCLUDE_DIRS
	NAMES rapidjson/rapidjson.h
	PATHS ${RAPIDJSON_INCLUDE_DIR}
	DOC "Include directory for the RapidJSON library"
)

mark_as_advanced(RAPIDJSON_INCLUDE_DIRS)

if(RAPIDJSON_INCLUDE_DIRS)
	set(RAPIDJSON_FOUND TRUE)
endif()

mark_as_advanced(RAPIDJSON_FOUND)

if(RAPIDJSON_USE_SSE42)
	set(RAPIDJSON_CXX_FLAGS "-DRAPIDJSON_SSE42")
	if(MSVC)
		set(RAPIDJSON_CXX_FLAGS "${RAPIDJSON_CXX_FLAGS} /arch:SSE4.2")
	else()
		set(RAPIDJSON_CXX_FLAGS "${RAPIDJSON_CXX_FLAGS} -msse4.2")
	endif()
else()
	if(RAPIDJSON_USE_SSE2)
		set(RAPIDJSON_CXX_FLAGS "-DRAPIDJSON_SSE2")
		if(MSVC)
			set(RAPIDJSON_CXX_FLAGS "${RAPIDJSON_CXX_FLAGS} /arch:SSE2")
		else()
			set(RAPIDJSON_CXX_FLAGS "${RAPIDJSON_CXX_FLAGS} -msse2")
		endif()
	endif()
endif()

mark_as_advanced(RAPIDJSON_CXX_FLAGS)

if(RAPIDJSON_FOUND)
	if(NOT RAPIDJSON_FIND_QUIETLY)
		message(STATUS "Found RapidJSON header files in ${RAPIDJSON_INCLUDE_DIRS}")
		if(DEFINED RAPIDJSON_CXX_FLAGS)
			message(STATUS "Found RapidJSON C++ extra compilation flags: ${RAPIDJSON_CXX_FLAGS}")
		endif()
	endif()
elseif(RAPIDJSON_FIND_REQUIRED)
	message(FATAL_ERROR "Could not find RapidJSON")
endif()
