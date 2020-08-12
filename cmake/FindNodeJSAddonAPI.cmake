#
# CMake Find NodeJS Addon API by Parra Studios
# Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find NodeJS Addon API include paths
#
# NODEJS_ADDON_API_FOUND - True if NodeJS Addon API was found
# NODEJS_ADDON_API_INCLUDE_DIR - NodeJS Addon API headers path

# Prevent vervosity if already included
if(NODEJS_ADDON_API_FOUND)
	set(NODEJS_ADDON_FIND_QUIETLY TRUE)
endif()

# Include package manager
include(FindPackageHandleStandardArgs)

set(NODEJS_EXECUTABLE_ONLY ON)

find_package(NodeJS)

if(NOT NODEJS_FOUND)
	message(STATUS "NodeJS executable not found")
	return()
endif()

# Get the includes
execute_process(
	COMMAND ${NODEJS_EXECUTABLE} -p "require('node-addon-api').include"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE NODEJS_ADDON_API_INCLUDE_DIR
	ERROR_VARIABLE NODEJS_ADDON_API_ERROR
)

string(REPLACE "\n" "" NODEJS_ADDON_API_INCLUDE_DIR ${NODEJS_ADDON_API_INCLUDE_DIR})
string(REPLACE "\"" "" NODEJS_ADDON_API_INCLUDE_DIR ${NODEJS_ADDON_API_INCLUDE_DIR})

if(NODEJS_ADDON_API_ERROR)
	message(FATAL_ERROR "Node Addon API includes could not be found. Include: ${NODEJS_ADDON_API_INCLUDE_DIR}. Error: ${NODEJS_ADDON_API_ERROR}")
	return()
else()
	message(STATUS "NODEJS_ADDON_API_INCLUDE_DIR: ${NODEJS_ADDON_API_INCLUDE_DIR}")
endif()

find_package_handle_standard_args(NODEJS_ADDON_API
	REQUIRED_VARS NODEJS_ADDON_API_INCLUDE_DIR
)

mark_as_advanced(NODEJS_ADDON_API_INCLUDE_DIR)
