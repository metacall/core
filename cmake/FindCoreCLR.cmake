#
#	CMake Find CoreCLR NET Engine by Parra Studios
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	CMake script to find CoreCLR NET Engine.
#

# Find CoreCLR library and include paths
#
# CORECLR_FOUND - True if CoreCLR was found
# CORECLR_INCLUDE_DIR - CoreCLR headers path
# CORECLR_LIBRARIES - List of CoreCLR libraries
# CORECLR_CGINFO - List of CoreCLR libraries



# Prevent vervosity if already included
if(CORECLR_FOUND)
	set(CORECLR_FIND_QUIETLY TRUE)
endif()

if(UNIX)
	set(CORECLR_FOUND 1)
	return()
endif()

# Include package manager
include(FindPackageHandleStandardArgs)

set(CORECLR_FOUND 0)

set(CORECLR_ROOT_REPOSITORY_PATH "" CACHE PATH "CoreCLR repository path")

if(NOT CORECLR_ROOT_REPOSITORY_PATH STREQUAL "")
	set(CORECLR_INCLUDE_DIR "${CORECLR_ROOT_REPOSITORY_PATH}/src")
	set(CORECLR_FOUND 1)
endif()
