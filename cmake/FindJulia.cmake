#
#	CMake Find Julia Runtime by Parra Studios
#	CMake script to find Julia runtime.
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

# Find Julia runtime library and include paths
#
# JULIA_FOUND - True if Julia runtime was found
# JULIA_EXECUTABLE - Path to the Julia executable
# JULIA_INCLUDE_DIRS - Julia headers path
# JULIA_LIBRARY - List of Julia libraries
# JULIA_LIBRARY_DIR - Folder containing Julia libraries
# JULIA_VERSION - Julia version string
# JULIA_VERSION_MAJOR - Julia major version
# JULIA_VERSION_MINOR - Julia minor version
# JULIA_VERSION_PATCH - Julia patch version
# JULIA_HOME - Folder containing Julia libraries
# JULIA_LLVM_VERSION - Version of LLVM that Julia is using
# JULIA_WORD_SIZE - Whether Julia supports 32 or 64 bits
#
# Options
#
# JULIA_DIR - Defines an alternative search path where Julia runtime is located
#

# Prevent vervosity if already included
if(JULIA_FOUND)
	set(JULIA_FIND_QUIETLY TRUE)
endif()

# Julia search path
option(JULIA_DIR "Defines an alternative search path where Julia runtime is located." OFF)

if(JULIA_DIR)
	list(APPEND CMAKE_PREFIX_PATH ${JULIA_DIR})
endif()

# Julia Executable
find_program(JULIA_EXECUTABLE julia DOC "Julia executable")

# Julia Version
set(JULIA_VERSION_RESULT 1)

if(JULIA_EXECUTABLE)
	execute_process(
		COMMAND "${JULIA_EXECUTABLE}" --startup-file=no --version
		OUTPUT_VARIABLE JULIA_VERSION_STRING
		RESULT_VARIABLE JULIA_VERSION_RESULT
	)
endif()

if(JULIA_VERSION_RESULT NOT EQUAL 0)
	find_file(JULIA_VERSION_INCLUDE
		julia_version.h
		PATH_SUFFIXES include/julia
	)
	file(READ ${JULIA_VERSION_INCLUDE} JULIA_VERSION_STRING)
	string(REGEX MATCH "JULIA_VERSION_STRING.*" JULIA_VERSION_STRING ${JULIA_VERSION_STRING})
endif()

string(
	REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1"
	JULIA_VERSION_STRING "${JULIA_VERSION_STRING}"
)

string(REPLACE "." ";" JULIA_VERSION_LIST ${JULIA_VERSION_STRING})
list(GET JULIA_VERSION_LIST 0 JULIA_VERSION_MAJOR)
list(GET JULIA_VERSION_LIST 1 JULIA_VERSION_MINOR)
list(GET JULIA_VERSION_LIST 2 JULIA_VERSION_PATCH)

set(JULIA_VERSION ${JULIA_VERSION_STRING} CACHE STRING "Julia version string")

if(${JULIA_VERSION} VERSION_LESS "0.7.0")
	set(JULIA_HOME_NAME "JULIA_HOME")
else()
	set(JULIA_HOME_NAME "Sys.BINDIR")
	set(JULIA_USING_LIBDL "using Libdl")
endif()

# Julia Headers
if(JULIA_DIR)
	set(JULIA_INCLUDE_DIRS ${JULIA_DIR}/include/julia CACHE STRING "Location of Julia include files")
elseif(JULIA_EXECUTABLE)
	execute_process(
		COMMAND ${JULIA_EXECUTABLE} --startup-file=no -E "julia_include_dir = joinpath(match(r\"(.*)(bin)\",${JULIA_HOME_NAME}).captures[1],\"include\",\"julia\")\n
			if !isdir(julia_include_dir)  # then we're running directly from build\n
			julia_base_dir_aux = splitdir(splitdir(${JULIA_HOME_NAME})[1])[1]  # useful for running-from-build\n
			julia_include_dir = joinpath(julia_base_dir_aux, \"usr\", \"include\" )\n
			julia_include_dir *= \";\" * joinpath(julia_base_dir_aux, \"src\", \"support\" )\n
			julia_include_dir *= \";\" * joinpath(julia_base_dir_aux, \"src\" )\n
			end\n
			julia_include_dir"
		OUTPUT_VARIABLE JULIA_INCLUDE_DIRS
	)

	string(REGEX REPLACE "\"" "" JULIA_INCLUDE_DIRS "${JULIA_INCLUDE_DIRS}")
	string(REGEX REPLACE "\n" "" JULIA_INCLUDE_DIRS "${JULIA_INCLUDE_DIRS}")

	set(JULIA_INCLUDE_DIRS ${JULIA_INCLUDE_DIRS} CACHE STRING "Location of Julia include files")
else()
	find_path(JULIA_INCLUDE_DIRS
		NAMES julia.h
		PATHS "/usr/include/julia"
		DOC "Location of Julia include files"
	)
	get_filename_component(JULIA_INCLUDE_DIRS ${JULIA_INCLUDE_DIRS} DIRECTORY)
endif()

# Julia Libraries
if(JULIA_EXECUTABLE)
	execute_process(
		COMMAND ${JULIA_EXECUTABLE} --startup-file=no -E "${JULIA_USING_LIBDL}\nabspath(Libdl.dlpath((ccall(:jl_is_debugbuild, Cint, ()) != 0) ? \"libjulia-debug\" : \"libjulia\"))"
		OUTPUT_VARIABLE JULIA_LIBRARY
	)

	string(REGEX REPLACE "\"" "" JULIA_LIBRARY "${JULIA_LIBRARY}")
	string(REGEX REPLACE "\n" "" JULIA_LIBRARY "${JULIA_LIBRARY}")
	string(STRIP "${JULIA_LIBRARY}" JULIA_LIBRARY)

	if(WIN32)
		get_filename_component(JULIA_LIBRARY_DIR ${JULIA_LIBRARY} DIRECTORY)
		get_filename_component(JULIA_LIBRARY_DIR ${JULIA_LIBRARY_DIR} DIRECTORY)
		find_library(WIN32_JULIA_LIBRARY
			NAMES libjulia.dll.a
			PATHS "${JULIA_LIBRARY_DIR}/lib"
			NO_DEFAULT_PATH
		)
		set(JULIA_LIBRARY "${WIN32_JULIA_LIBRARY}")
	endif()

	set(JULIA_LIBRARY "${JULIA_LIBRARY}" CACHE PATH "Julia library")
else()
	set(JULIA_LIBRARY_NAMES
		libjulia.so.${JULIA_VERSION_MAJOR}.${JULIA_VERSION_MINOR}.${JULIA_VERSION_PATCH}
		libjulia.so.${JULIA_VERSION_MAJOR}.${JULIA_VERSION_MINOR}
		libjulia.so.${JULIA_VERSION_MAJOR}
		libjulia.so
		libjulia.${JULIA_VERSION}.dylib
		julia
		libjulia
		libjulia.dll.a
	)
	find_library(JULIA_LIBRARY
		NAMES ${JULIA_LIBRARY_NAMES}
		DOC "Location of Julia library"
		CMAKE_FIND_ROOT_PATH_BOTH
	)
endif()

get_filename_component(JULIA_LIBRARY_DIR ${JULIA_LIBRARY} DIRECTORY)

# Additional Julia information
if(JULIA_EXECUTABLE)
	# Julia Home
	execute_process(
		COMMAND ${JULIA_EXECUTABLE} --startup-file=no -E "${JULIA_HOME_NAME}"
		OUTPUT_VARIABLE JULIA_HOME
	)

	string(REGEX REPLACE "\"" "" JULIA_HOME "${JULIA_HOME}")
	string(REGEX REPLACE "\n" "" JULIA_HOME "${JULIA_HOME}")


	# LLVM Version
	execute_process(
		COMMAND ${JULIA_EXECUTABLE} --startup-file=no -E "Base.libllvm_version"
		OUTPUT_VARIABLE JULIA_LLVM_VERSION
	)

	string(REGEX REPLACE "\"" "" JULIA_LLVM_VERSION "${JULIA_LLVM_VERSION}")
	string(REGEX REPLACE "\n" "" JULIA_LLVM_VERSION "${JULIA_LLVM_VERSION}")

	# Word Size
	execute_process(
		COMMAND ${JULIA_EXECUTABLE} --startup-file=no -E "Sys.WORD_SIZE"
		OUTPUT_VARIABLE JULIA_WORD_SIZE
	)
	string(REGEX REPLACE "\n" "" JULIA_WORD_SIZE "${JULIA_WORD_SIZE}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(JULIA
	REQUIRED_VARS	JULIA_LIBRARY JULIA_LIBRARY_DIR JULIA_INCLUDE_DIRS JULIA_EXECUTABLE JULIA_VERSION
	VERSION_VAR		JULIA_VERSION
	FAIL_MESSAGE	"Julia not found"
)
