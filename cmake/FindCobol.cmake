#
#	CMake Find GNU/Cobol by Parra Studios
#	CMake script to find GNU/Cobol compiler and runtime.
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

# Find GNU/Cobol library and include paths
#
# COBOL_FOUND - True if GNU/Cobol was found
# COBOL_EXECUTABLE - GNU/Cobol compiler executable path
# COBOL_VERSION - GNU/Cobol installed version
# COBOL_INCLUDE_DIR - GNU/Cobol headers path
# COBOL_LIBRARY - List of GNU/Cobol libraries

# Prevent vervosity if already included
if(COBOL_EXECUTABLE)
	set(COBOL_FIND_QUITELY TRUE)
endif()

option(COBOL_CMAKE_DEBUG "Print paths for debugging GNU/Cobol dependencies." OFF)

include(FindPackageHandleStandardArgs)

# Find GNU/Cobol compiler executable
find_program(COBOL_EXECUTABLE
	NAMES cobc cobc.exe
	PATH_SUFFIXES bin
	DOC "GNU/Cobol Compiler"
)

# Find GNU/Cobol version
if(COBOL_EXECUTABLE)
	execute_process(COMMAND ${COBOL_EXECUTABLE} --version
		OUTPUT_VARIABLE COBOL_VERSION_TAG
		RESULT_VARIABLE COBOL_VERSION_TAG_RESULT
	)

	if(COBOL_VERSION_TAG_RESULT EQUAL 0)
		# Get first line
		string(REGEX REPLACE ";" "\\\\;" COBOL_VERSION_TAG "${COBOL_VERSION_TAG}")
		string(REGEX REPLACE "\n" ";" COBOL_VERSION_TAG "${COBOL_VERSION_TAG}")
		list(GET COBOL_VERSION_TAG 0 COBOL_VERSION_TAG)

		# Get version
		string(REGEX REPLACE " " ";" COBOL_VERSION_TAG "${COBOL_VERSION_TAG}")
		list(GET COBOL_VERSION_TAG 2 COBOL_VERSION_TAG)

		# Set version constants
		set(COBOL_VERSION "${COBOL_VERSION_TAG}")
		string(REPLACE "." ";" COBOL_VERSION_LIST "${COBOL_VERSION}")
		list(GET COBOL_VERSION_LIST 0 COBOL_VERSION_MAJOR)
		list(GET COBOL_VERSION_LIST 1 COBOL_VERSION_MINOR)
		list(GET COBOL_VERSION_LIST 2 COBOL_VERSION_PATCH)
	endif()
endif()

# Find GNU/Cobol include directory
set(COBOL_INCLUDE_PATHS
	/usr
	/usr/local
)

set(COBOL_INCLUDE_SUFFIXES
	include
)

find_path(COBOL_INCLUDE_DIR
	NAMES libcob.h
	PATHS ${COBOL_INCLUDE_PATHS}
	PATH_SUFFIXES ${COBOL_INCLUDE_SUFFIXES}
	DOC "GNU/Cobol Headers"
)

# Find library
if(WIN32)
	set(COBOL_LIBRARY_SUFFIX "dll")
elseif(APPLE)
	set(COBOL_LIBRARY_SUFFIX "dylib")
else()
	set(COBOL_LIBRARY_SUFFIX "so")
endif()

set(COBOL_LIBRARY_NAMES
	cob.dll
	cob.dylib
	libcob.so
	libcob.so.${COBOL_VERSION_MAJOR}
	libcob.so.${COBOL_VERSION}
)

set(COBOL_LIBRARY_PATH
	/usr/lib
	/usr/local/lib
)

find_library(COBOL_LIBRARY
	NAMES ${COBOL_LIBRARY_NAMES}
	PATHS ${COBOL_LIBRARY_PATH}
	DOC "GNU/Cobol Runtime Library"
)

# Show debug info if enabled
if(COBOL_CMAKE_DEBUG)
	message(STATUS "COBOL_EXECUTABLE: ${COBOL_LIBRARY_NAMES}")
	message(STATUS "COBOL_VERSION: ${COBOL_VERSION}")
	message(STATUS "COBOL_INCLUDE_DIR: ${COBOL_INCLUDE_DIR}")
	message(STATUS "COBOL_LIBRARY: ${COBOL_LIBRARY}")
	message(STATUS "COBOL_LIBRARY_SUFFIX: ${COBOL_LIBRARY_SUFFIX}")
endif()

# Set standard args
find_package_handle_standard_args(COBOL
	REQUIRED_VARS COBOL_EXECUTABLE COBOL_INCLUDE_DIR COBOL_LIBRARY COBOL_LIBRARY_SUFFIX
	VERSION_VAR COBOL_VERSION
)

# Mark cmake module as advanced
mark_as_advanced(COBOL_EXECUTABLE COBOL_INCLUDE_DIR COBOL_LIBRARY)
