#
#	CMake Find Clang library by Parra Studios
#	CMake script to find Clang C API library.
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

# Find Clang C API library and include paths
#
# LibClang_FOUND - True if Clang library was found
# LibClang_INCLUDE_DIR - Clang headers path
# LibClang_LIBRARY - Clang C API library

# Prevent vervosity if already included
if(LibClang_FOUND)
	set(LibClang_FIND_QUITELY TRUE)
endif()

option(LibClang_CMAKE_DEBUG "Print paths for debugging LibClang dependencies." OFF)

# Define what version to search for
if(LibClang_FIND_VERSION)
	set(LibClang_VERSION_LIST ${LibClang_FIND_VERSION})
else()
	set(LibClang_VERSION_LIST 13 12 11 10 9 8 7 6.0 5.0 4.0 3.9 3.8)
endif()

macro(_libclang_generate_search_paths template result)
	set(${result})
	foreach(version ${LibClang_VERSION_LIST})
		string(REPLACE "VERSION" "${version}" output_path "${template}")
		list(APPEND ${result} ${output_path})
	endforeach()
endmacro()

_libclang_generate_search_paths("/usr/lib/llvm-VERSION/lib/" LibClang_LIBRARY_PATHS)

include(FindPackageHandleStandardArgs)

# Find Clang C API Library
_libclang_generate_search_paths("/usr/lib/llvm-VERSION/lib/" LibClang_LIBRARY_PATHS)

find_library(LibClang_LIBRARY
	NAMES clang
	PATHS ${LibClang_LIBRARY_PATHS}
)

# Find Clang C API Headers
_libclang_generate_search_paths("/usr/lib/llvm-VERSION/include/clang-c" LibClang_INCLUDE_PATHS)

set(LibClang_INCLUDE_HEADERS
	BuildSystem.h
	CXCompilationDatabase.h
	CXErrorCode.h
	CXString.h
	Documentation.h
	Index.h
	Platform.h
)

find_path(LibClang_INCLUDE_DIR
	NAMES ${LibClang_INCLUDE_HEADERS}
	PATHS ${LibClang_INCLUDE_PATHS}
)

get_filename_component(LibClang_INCLUDE_DIR ${LibClang_INCLUDE_DIR} DIRECTORY)

# Define LibClang cmake module
find_package_handle_standard_args(LibClang DEFAULT_MSG LibClang_LIBRARY LibClang_INCLUDE_DIR)

# Mark cmake module as advanced
mark_as_advanced(LibClang_LIBRARY LibClang_INCLUDE_DIR)

if(LibClang_CMAKE_DEBUG)
	message(STATUS "LibClang_INCLUDE_DIRS: ${LibClang_INCLUDE_DIR}")
	message(STATUS "LibClang_LIBRARY: ${LibClang_LIBRARY}")
endif()
