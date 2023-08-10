#
#	CMake Install RapidJSON by Parra Studios
#	CMake script to install RapidJSON library.
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
# RAPIDJSON_CXX_FLAGS - Custom RapidJSON compilation flags.
# RAPIDJSON_INCLUDE_DIRS - A list of directories where the RapidJSON headers are located.

if(NOT RAPIDJSON_FOUND OR USE_BUNDLED_RAPIDJSON)
	if(NOT RAPIDJSON_VERSION OR USE_BUNDLED_RAPIDJSON)
		set(RAPIDJSON_VERSION 232389d)
		set(RAPIDJSON_URL_MD5 577d3495a07b66fcd4a2866c93831bc4)
	endif()

	ExternalProject_Add(rapid-json-depends
		DOWNLOAD_NAME				RapidJSON-${RAPIDJSON_VERSION}.tar.gz
		URL							https://github.com/Tencent/rapidjson/archive/${RAPIDJSON_VERSION}.tar.gz
		URL_MD5						${RAPIDJSON_URL_MD5}
		CMAKE_ARGS					-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
									-DRAPIDJSON_BUILD_DOC=Off
									-DRAPIDJSON_BUILD_EXAMPLES=Off
									-DRAPIDJSON_BUILD_TESTS=Off
		TEST_COMMAND				""
		DOWNLOAD_EXTRACT_TIMESTAMP	OFF
	)

	ExternalProject_Get_Property(rapid-json-depends INSTALL_DIR)

	set(RAPIDJSON_ROOT_DIR		${INSTALL_DIR})
	set(RAPIDJSON_INCLUDE_DIRS	${RAPIDJSON_ROOT_DIR}/include)
	set(RAPIDJSON_FOUND			TRUE)

	mark_as_advanced(RAPIDJSON_INCLUDE_DIRS)

	message(STATUS "Installing RapidJSON v${RAPIDJSON_VERSION}")
endif()
