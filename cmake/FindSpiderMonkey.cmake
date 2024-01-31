#
#	CMake Find Mozilla SpiderMonkey JavaScript Engine by Parra Studios
#	CMake script to find Mozilla SpiderMonkey Javascript Engine.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Find the SpiderMonkey header files and libraries
#
#  SPIDERMONKEY_INCLUDE_DIR	- where to find jsapi.h, etc.
#  SPIDERMONKEY_LIBRARY		- List of libraries when using libjs.
#  SPIDERMONKEY_FOUND		  - True if SpiderMonkey found.
#  SPIDERMONKEY_THREADSAFE	 - True if SpiderMonkey is compiled with multi threading support.
#
#  SPIDERMONKEY_VERSION_STRING - The version of SpiderMonkey library found (x.y.z)
#  SPIDERMONKEY_VERSION_MAJOR  - The major version of SpiderMonkey library
#  SPIDERMONKEY_VERSION_MINOR  - The minor version of SpiderMonkey library
#  SPIDERMONKEY_VERSION_PATCH  - The patch version of SpiderMonkey library

include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(FindPackageHandleStandardArgs)

# Prevent vervosity if already included
if(SPIDERMONKEY_FOUND)
	set(SPIDERMONKEY_FIND_QUIETLY TRUE)
endif()

# SpiderMonkey search paths
set(SPIDERMONKEY_PATHS
	${SPIDERMONKEY_ROOT}
	$ENV{SPIDERMONKEY_ROOT}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/usr/freeware
)

# SpiderMonkey headers
set(SPIDERMONKEY_HEADERS jsapi.h js/RequiredDefines.h)

# SpiderMonkey include suffix paths
set(SPIDERMONKEY_INCLUDE_SUFFIX_PATHS include include/js include/mozjs-48a1)

# Find SpiderMonkey include path
find_path(SPIDERMONKEY_INCLUDE_DIR ${SPIDERMONKEY_HEADERS}
	PATHS ${SPIDERMONKEY_PATHS}
	PATH_SUFFIXES ${SPIDERMONKEY_INCLUDE_SUFFIX_PATHS}
	DOC "Mozilla SpiderMonkey JavaScript Engine Headers"
)

# SpiderMonkey libs
set(SPIDERMONKEY_LIBRARY_NAMES mozjs185 mozjs-1.9.2 mozjs-48a1 mozjs js185 js js32 js3250)

# Find SpiderMonkey base library debug
find_library(SPIDERMONKEY_LIBRARY
	NAMES ${SPIDERMONKEY_LIBRARY_NAMES}
	PATHS ${SPIDERMONKEY_PATHS}
	PATH_SUFFIXES lib
	DOC "Mozilla SpiderMonkey JavaScript Engine Library"
)

set(SPIDERMONKEY_LIBRARIES ${SPIDERMONKEY_LIBRARY})

list(APPEND CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
list(APPEND CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
list(APPEND CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})

check_cxx_source_compiles(
	"#include <jsapi.h>
	int main()
	{
		JSRuntime *rt = JS_NewRuntime(8L * 1024L * 1024L);

		if (rt != NULL)
		{
			return 0;
		}

		return 1;
	}"

	SPIDERMONKEY_BUILDS
)

find_package_handle_standard_args(SpiderMonkey DEFAULT_MSG SPIDERMONKEY_LIBRARIES SPIDERMONKEY_INCLUDE_DIR) # SPIDERMONKEY_BUILDS)

if(SPIDERMONKEY_FOUND)
	set(CMAKE_REQUIRED_INCLUDES ${SPIDERMONKEY_INCLUDE_DIR})
	set(CMAKE_REQUIRED_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS})
	set(CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})


	find_path(SPIDERMONKEY_JS_CONFIG_HEADER_PATH js-config.h
		PATHS ${SPIDERMONKEY_PATHS}
		PATH_SUFFIXES ${SPIDERMONKEY_INCLUDE_SUFFIX_PATHS}
		DOC "Mozilla SpiderMonkey JavaScript Engine Config Header"
	)

	find_path(SPIDERMONKEY_JS_VERSION_HEADER_PATH jsversion.h
		PATHS ${SPIDERMONKEY_PATHS}
		PATH_SUFFIXES ${SPIDERMONKEY_INCLUDE_SUFFIX_PATHS}
		DOC "Mozilla SpiderMonkey JavaScript Engine Version Header"
	)

	check_include_file_cxx("${SPIDERMONKEY_JS_CONFIG_HEADER_PATH}/js-config.h" SPIDERMONKEY_JS_CONFIG_HEADER)
	check_include_file_cxx("${SPIDERMONKEY_JS_CONFIG_HEADER_PATH}/jsversion.h" SPIDERMONKEY_JS_VERSION_HEADER)

	if(SPIDERMONKEY_JS_VERSION_HEADER)
		file(STRINGS "${SPIDERMONKEY_JS_VERSION_HEADER_PATH}/jsversion.h" JSVERSION REGEX "^#define JS_VERSION [^\"]*$")
		string(REGEX REPLACE "^.*JS_VERSION ([0-9]+)*$" "\\1" SPIDERMONKEY_VERSION_STRING "${JSVERSION}")
		string(REGEX REPLACE "^([0-9]).." "\\1" SPIDERMONKEY_VERSION_MAJOR  "${SPIDERMONKEY_VERSION_STRING}")
		string(REGEX REPLACE "^.([0-9])." "\\1" SPIDERMONKEY_VERSION_MINOR  "${SPIDERMONKEY_VERSION_STRING}")
		string(REGEX REPLACE "^..([0-9])" "\\1" SPIDERMONKEY_VERSION_PATCH  "${SPIDERMONKEY_VERSION_STRING}")
	endif()

	if(NOT SPIDERMONKEY_JS_CONFIG_HEADER)
		check_cxx_source_runs(
			"#include <stdio.h>

			extern \"C\" void js_GetCurrentThread();

			int main()
			{
				printf(\"%p\", (void *)js_GetCurrentThread);

				return ((void *)js_GetCurrentThread != NULL) ? 0 : 1;
			}"

			SPIDERMONKEY_THREADSAFE
		)

		if(SPIDERMONKEY_THREADSAFE)
			set(SPIDERMONKEY_DEFINITIONS ${SPIDERMONKEY_DEFINITIONS} -DJS_THREADSAFE)
		endif()
	else()
		file(STRINGS "${SPIDERMONKEY_JS_CONFIG_HEADER_PATH}/js-config.h" JS_THREADSAFE REGEX "^#define JS_THREADSAFE [^\"]*$")

		if(${JS_THREADSAFE} MATCHES "1")
			set(SPIDERMONKEY_THREADSAFE "TRUE")
		endif()
	endif()

	if(NOT SPIDERMONKEY_JS_VERSION_HEADER)
		message(FATAL_ERROR "SpiderMonkey include file jsversion.h not found.")
	endif()

	# Removed in newest versions of SpiderMonkey
	#if(NOT SPIDERMONKEY_THREADSAFE)
	#	message(FATAL_ERROR "SpiderMonkey must be built with JS_THREADSAFE")
	#endif()

	get_filename_component(SPIDERMONKEY_LIBDIR ${SPIDERMONKEY_LIBRARY} PATH)

	link_directories(${SPIDERMONKEY_LIBDIR})
endif()

list(REMOVE_ITEM CMAKE_REQUIRED_LIBRARIES ${SPIDERMONKEY_LIBRARY})
list(REMOVE_DUPLICATES CMAKE_REQUIRED_LIBRARIES)

mark_as_advanced(SPIDERMONKEY_INCLUDE_DIR SPIDERMONKEY_LIBRARY)
mark_as_advanced(SPIDERMONKEY_JS_CONFIG_HEADER_PATH)
