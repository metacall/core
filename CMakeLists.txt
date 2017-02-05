#
#	MetaCall Library by Parra Studios
#	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	A library for providing a foreing function interface calls.
#

#
# CMake options
#

# CMake version
cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

# Include cmake modules

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(WriterCompilerDetectionHeaderFound NOTFOUND)

include(GenerateExportHeader)
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
set(META_PROJECT_NAME			"metacall")
set(META_PROJECT_DESCRIPTION	"MetaCall Framework")
set(META_AUTHOR_ORGANIZATION	"Parra Studios")
set(META_AUTHOR_DOMAIN			"https://parrastudios.com/")
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
option(BUILD_DISTRIBUTABLE_LIBS	"Build all libraries into a single compilation unit."		ON)
option(OPTION_SELF_CONTAINED	"Create a self-contained install with all dependencies."	OFF)
option(OPTION_BUILD_TESTS		"Build tests."												ON)
option(OPTION_BUILD_DOCS		"Build documentation."										OFF)
option(OPTION_BUILD_EXAMPLES	"Build examples."											ON)
option(OPTION_BUILD_PLUGINS		"Build plugins."											ON)
option(OPTION_BUILD_SCRIPTS		"Build scripts."											ON)
option(OPTION_BUILD_PORTS		"Build ports."												ON)
option(OPTION_BUILD_CONFIGS		"Build configurations."										ON)

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
# Compiler settings and options
#

include(CompileOptions)

#
# Deployment/installation setup
#

# Get project name
set(project ${META_PROJECT_NAME})

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
set(CMAKE_SKIP_BUILD_RPATH			FALSE) # Add absolute path to all dependencies for BUILD
set(CMAKE_BUILD_WITH_INSTALL_RPATH	FALSE) # Use CMAKE_INSTALL_RPATH for INSTALL
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE) # Do NOT add path to dependencies for INSTALL

if(NOT SYSTEM_DIR_INSTALL)
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
# Project module names
#

set(MODULE_NAMES
	version
	preprocessor
	format
	log
	adt
	filesystem
	dynlink
	reflect
	configuration
	loader
	metacall
)

#
# Project module includes
#

add_subdirectory(source)
add_subdirectory(docs)
add_subdirectory(deploy)

#
# Project configuration generation
#

set(PROJECT_CONFIGURATION "metacall-config.cmake")

set(PROJECT_CONFIGURATION_PATH "${PROJECT_BINARY_DIR}/${PROJECT_CONFIGURATION}")

configure_file("${PROJECT_CONFIGURATION}.in" "${PROJECT_CONFIGURATION_PATH}" @ONLY)

#
# Deployment (global project files)
#

# Install version file
install(FILES "${PROJECT_BINARY_DIR}/VERSION" DESTINATION ${INSTALL_ROOT} COMPONENT runtime)

# Install cmake find script for the project
install(FILES ${PROJECT_CONFIGURATION_PATH} DESTINATION ${INSTALL_ROOT} COMPONENT dev)

# Install the project meta files
install(FILES AUTHORS   DESTINATION ${INSTALL_ROOT} COMPONENT runtime)
install(FILES LICENSE   DESTINATION ${INSTALL_ROOT} COMPONENT runtime)
install(FILES README	DESTINATION ${INSTALL_ROOT} COMPONENT runtime)

# Install runtime data
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data DESTINATION ${INSTALL_DATA} COMPONENT runtime)
