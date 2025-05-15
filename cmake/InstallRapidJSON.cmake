#
#	CMake Install RapidJSON by Parra Studios
#	CMake script to install RapidJSON library.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
		set(RAPIDJSON_VERSION 24b5e7a8b27f42fa16b96fc70aade9106cf7102f)
	endif()

	ExternalProject_Add(rapid-json-depends
		GIT_REPOSITORY		"https://github.com/Tencent/rapidjson.git"
		GIT_TAG				"${RAPIDJSON_VERSION}"
		BUILD_COMMAND		""
		CONFIGURE_COMMAND	""
		INSTALL_COMMAND		""
		TEST_COMMAND		""
	)

	ExternalProject_Get_Property(rapid-json-depends SOURCE_DIR)

	set(RAPIDJSON_ROOT_DIR		${SOURCE_DIR})
	set(RAPIDJSON_INCLUDE_DIRS	${RAPIDJSON_ROOT_DIR}/include)
	set(RAPIDJSON_FOUND			TRUE)

	mark_as_advanced(RAPIDJSON_INCLUDE_DIRS)

	message(STATUS "Installing RapidJSON v${RAPIDJSON_VERSION}")
endif()
