#
#	CMake Install Tiny C Compiler by Parra Studios
#	CMake script to install TCC library.
#
#	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
# LIBTCC_FOUND - True if TCC library was found
# LIBTCC_INCLUDE_DIR - TCC headers path
# LIBTCC_LIBRARY - List of TCC libraries

if(LIBTCC_FOUND)
	return()
endif()

include(ProcessorCount)
ProcessorCount(N)

include(Platform)

# Configure
if(PROJECT_OS_FAMILY EQUAL unix)
	set(LIBTCC_CONFIGURE "./configure --disable-static --with-libgcc --with-selinux")
elseif(PROJECT_OS_FAMILY EQUAL win32)
	if(PROJECT_OS_NAME STREQUAL "MinGW")
		set(LIBTCC_CONFIGURE "./configure --config-mingw32 --disable-static --with-libgcc --with-selinux")
	else()
		set(LIBTCC_CONFIGURE "")
	endif()
else()
	message(FATAL_ERROR "TCC library not implemented in this platform")
endif()

# Build
if(PROJECT_OS_BSD)
	set(LIBTCC_BUILD "gmake -j${N}")
elseif(PROJECT_OS_FAMILY EQUAL unix)
	set(LIBTCC_BUILD "make -j${N}")
elseif(PROJECT_OS_FAMILY EQUAL win32)
	if(PROJECT_OS_NAME STREQUAL "MinGW")
		set(LIBTCC_CONFIGURE "make -j${N}")
	else()
		set(LIBTCC_CONFIGURE "./win32/build-tcc.bat")
	endif()
else()
	message(FATAL_ERROR "TCC library not implemented in this platform")
endif()

ExternalProject_Add(libtcc-depends
	DOWNLOAD_NAME	tinycc.tar.gz
	URL				https://repo.or.cz/tinycc.git/snapshot/da11cf651576f94486dbd043dbfcde469e497574.tar.gz
	URL_MD5			cc0cde5f454fa3a8c068da95edaaea86
	# TODO: Install prefix, configure and build commands
	TEST_COMMAND	""
)

# TODO
ExternalProject_Get_Property(libtcc-json-depends INSTALL_DIR)

set(RAPIDJSON_ROOT_DIR		${INSTALL_DIR})
set(RAPIDJSON_INCLUDE_DIRS	${RAPIDJSON_ROOT_DIR}/include)
set(RAPIDJSON_FOUND			TRUE)

mark_as_advanced(RAPIDJSON_INCLUDE_DIRS)

message(STATUS "Installing RapidJSON v${RAPIDJSON_VERSION}")
