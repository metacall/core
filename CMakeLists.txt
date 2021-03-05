#
#	MetaCall Library by Parra Studios
#	A library for providing a foreing function interface calls.
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

#
# CMake options
#

# CMake version
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# Include cmake modules

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(WriterCompilerDetectionHeaderFound NOTFOUND)

include(ExternalProject)

# This module is only available with CMake >=3.1, so check whether it could be found
include(WriteCompilerDetectionHeader OPTIONAL RESULT_VARIABLE WriterCompilerDetectionHeaderFound)

include(GetGitRevisionDescription)
include(Custom)

# Set policies
set_policy(CMP0028 NEW) # ENABLE CMP0028: Double colon in target name means ALIAS or IMPORTED target.
set_policy(CMP0054 NEW) # ENABLE CMP0054: Only interpret if() arguments as variables or keywords when unquoted.
set_policy(CMP0042 NEW) # ENABLE CMP0042: MACOSX_RPATH is enabled by default.
set_policy(CMP0063 NEW) # ENABLE CMP0063: Honor visibility properties for all target types.

#
# Project description and (meta) information
#

# Get git revision
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
string(SUBSTRING "${GIT_SHA1}" 0 12 GIT_REV)

# Meta information about the project
set(META_PROJECT_NAME			"MetaCall")
set(META_PROJECT_DESCRIPTION	"A library for providing inter-language foreign function interface calls")
set(META_AUTHOR_ORGANIZATION	"MetaCall Inc.")
set(META_AUTHOR_DOMAIN			"https://metacall.io/")
set(META_AUTHOR_MAINTAINER		"vic798@gmail.com")
set(META_VERSION_MAJOR			"0")
set(META_VERSION_MINOR			"1")
set(META_VERSION_PATCH			"0")
set(META_VERSION_REVISION		"${GIT_REV}")
set(META_VERSION				"${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
set(META_NAME_VERSION			"${META_PROJECT_NAME} v${META_VERSION} (${META_VERSION_REVISION})")

#
# Project configuration options
#

# Project options
option(BUILD_SHARED_LIBS		"Build shared instead of static libraries."					ON)
option(OPTION_SELF_CONTAINED	"Create a self-contained install with all dependencies."	OFF)
option(OPTION_BUILD_TESTS		"Build tests."												ON)
option(OPTION_BUILD_BENCHMARKS	"Build benchmarks."											OFF)
option(OPTION_BUILD_DOCS		"Build documentation."										OFF)
option(OPTION_BUILD_EXAMPLES	"Build examples."											ON)
option(OPTION_BUILD_CLI			"Build CLIs."												ON)
option(OPTION_BUILD_LOADERS		"Build loaders."											ON)
option(OPTION_BUILD_SCRIPTS		"Build scripts."											ON)
option(OPTION_BUILD_SERIALS		"Build serials."											ON)
option(OPTION_BUILD_DETOURS		"Build detours."											ON)
option(OPTION_BUILD_PORTS		"Build ports."												OFF)
option(OPTION_BUILD_LOG_PRETTY	"Build logs in a human readable format."					ON)
option(OPTION_BUILD_PIC			"Build with position independent code."						ON)
option(OPTION_BUILD_SECURITY	"Build with stack-smashing protection and source fortify."	ON)
option(OPTION_BUILD_GUIX		"Disable all build system unreproductible operations."		OFF)
option(OPTION_FORK_SAFE			"Enable fork safety."										ON)
option(OPTION_THREAD_SAFE		"Enable thread safety."										OFF)
option(OPTION_COVERAGE			"Enable coverage."											OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Define the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

#
# Declare project
#

# Generate folders for IDE targets (e.g., VisualStudio solutions)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(IDE_FOLDER "")

# Declare project
project(${META_PROJECT_NAME} C CXX)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Create version file
file(WRITE "${PROJECT_BINARY_DIR}/VERSION" "${META_NAME_VERSION}")

#
# Define Export Headers
#

include(GenerateExportHeader)

# Patch export headers in order to produce "${target_upper}_EXPORTS" in API definition
function(GENERATE_EXPORT_HEADER)
	# This function is patched again in Portability, due to Haiku workaround, review it if this func gets removed
	set(target ${ARGV0})
	string(TOUPPER ${target} target_upper)
	set_target_properties(${target}
		PROPERTIES DEFINE_SYMBOL "${target_upper}_EXPORTS"
	)
	# When the function is redefined, the old function can be accessed through underscore
	_GENERATE_EXPORT_HEADER(${ARGN})
endfunction()

#
# Portability
#

include(Portability)

#
# Compiler settings and options
#

include(CompileOptions)

#
# Coverage
#

if(OPTION_COVERAGE)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/coverage")

	set(ENABLE_COVERAGE ON CACHE BOOL "Enable coverage build." FORCE)
	set(ENABLE_COVERAGE_ALL ON CACHE BOOL "Enable coverage build for all targets." FORCE)

	find_package(codecov)
endif()

#
# Deployment/installation setup
#

# Get project name
string(TOLOWER ${META_PROJECT_NAME} project)

# Check for system dir install
set(SYSTEM_DIR_INSTALL FALSE)
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
	set(SYSTEM_DIR_INSTALL TRUE)
endif()

# Installation paths
if(UNIX AND SYSTEM_DIR_INSTALL)
	# Install into the system (/usr/bin or /usr/local/bin)
	set(INSTALL_ROOT		"share/${project}")			# /usr/[local]/share/<project>
	set(INSTALL_CMAKE		"share/${project}/cmake")	# /usr/[local]/share/<project>/cmake
	set(INSTALL_EXAMPLES	"share/${project}")			# /usr/[local]/share/<project>
	set(INSTALL_DATA		"share/${project}")			# /usr/[local]/share/<project>
	set(INSTALL_BIN			"bin")						# /usr/[local]/bin
	set(INSTALL_SHARED		"lib")						# /usr/[local]/lib
	set(INSTALL_LIB			"lib")						# /usr/[local]/lib
	set(INSTALL_INCLUDE		"include")					# /usr/[local]/include
	set(INSTALL_DOC			"share/doc/${project}")		# /usr/[local]/share/doc/<project>
	set(INSTALL_SHORTCUTS	"share/applications")		# /usr/[local]/share/applications
	set(INSTALL_ICONS		"share/pixmaps")			# /usr/[local]/share/pixmaps
	set(INSTALL_INIT		"/etc/init")				# /etc/init (upstart init scripts)
else()
	# Install into local directory
	set(INSTALL_ROOT		".")						# ./
	set(INSTALL_CMAKE		"cmake")					# ./cmake
	set(INSTALL_EXAMPLES	".")						# ./
	set(INSTALL_DATA		".")						# ./
	set(INSTALL_BIN			".")						# ./
	set(INSTALL_SHARED		"lib")						# ./lib
	set(INSTALL_LIB			"lib")						# ./lib
	set(INSTALL_INCLUDE		"include")					# ./include
	set(INSTALL_DOC			"doc")						# ./doc
	set(INSTALL_SHORTCUTS	"misc")						# ./misc
	set(INSTALL_ICONS		"misc")						# ./misc
	set(INSTALL_INIT		"misc")						# ./misc
endif()

# Set runtime path
set(CMAKE_SKIP_BUILD_RPATH				FALSE)	# Add absolute path to all dependencies for BUILD
set(CMAKE_BUILD_WITH_INSTALL_RPATH		FALSE)	# Use CMAKE_INSTALL_RPATH for INSTALL
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH	TRUE)	# Add path to dependencies for INSTALL

if(SYSTEM_DIR_INSTALL)
	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}")
else()
	# Find libraries relative to binary
	if(APPLE)
		set(CMAKE_INSTALL_RPATH "@loader_path/../../../${INSTALL_LIB}")
	else()
		set(CMAKE_INSTALL_RPATH "$ORIGIN/${INSTALL_LIB}")
	endif()
endif()

#
# CTest configuration
#

if(OPTION_BUILD_TESTS)
	enable_testing()
endif()

#
# Project module includes
#

add_subdirectory(source)
add_subdirectory(docs)
add_subdirectory(deploy)

#
# Coverage evaluation (must run after all targets)
#

if(OPTION_COVERAGE)
	coverage_evaluate()
endif()

#
# Project configuration generation
#

set(EXPORT_MODULE_NAMES
	version
	preprocessor
	environment
	format
	threading
	log
	memory
	portability
	adt
	filesystem
	reflect
	dynlink
	detour
	serial
	configuration
	loader
	metacall
)

set(PROJECT_CONFIGURATION "metacall-config.cmake")
set(PROJECT_CONFIGURATION_VERSION "metacall-config-version.cmake")

set(PROJECT_CONFIGURATION_PATH "${PROJECT_BINARY_DIR}/${PROJECT_CONFIGURATION}")
set(PROJECT_CONFIGURATION_VERSION_PATH "${PROJECT_BINARY_DIR}/${PROJECT_CONFIGURATION_VERSION}")

configure_file("${PROJECT_CONFIGURATION}.in" "${PROJECT_CONFIGURATION_PATH}" @ONLY)
configure_file("${PROJECT_CONFIGURATION_VERSION}.in" "${PROJECT_CONFIGURATION_VERSION_PATH}" @ONLY)

#
# Deployment (global project files)
#

# Install version file
install(FILES "${PROJECT_BINARY_DIR}/VERSION" DESTINATION ${INSTALL_ROOT} COMPONENT runtime)

# Install cmake find script for the project
install(FILES ${PROJECT_CONFIGURATION_PATH} DESTINATION ${INSTALL_ROOT} COMPONENT dev)
install(FILES ${PROJECT_CONFIGURATION_VERSION_PATH} DESTINATION ${INSTALL_ROOT} COMPONENT dev)

# Install the project meta files
install(FILES AUTHORS	DESTINATION ${INSTALL_ROOT} COMPONENT runtime)
install(FILES LICENSE	DESTINATION ${INSTALL_ROOT} COMPONENT runtime)
install(FILES README.md	DESTINATION ${INSTALL_ROOT} COMPONENT runtime)

# # Install runtime data
# install(DIRECTORY ${PROJECT_SOURCE_DIR}/data DESTINATION ${INSTALL_DATA} COMPONENT runtime)
