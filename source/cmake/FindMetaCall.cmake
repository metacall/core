#
# CMake Find MetaCall library by Parra Studios
# Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find MetaCall library and include paths
#
# METACALL_FOUND - True if MetaCall library was found
# METACALL_INCLUDE_DIRS - MetaCall headers path
# METACALL_LIBRARIES - List of MetaCall libraries
# METACALL_DEFINITIONS - MetaCall definitions

# Prevent vervosity if already included
if(METACALL_FOUND)
	set(METACALL_FIND_QUITELY TRUE)
endif()

# Optional root directory
set(METACALL_ROOT_DIR "$ENV{WORKSPACE_ROOT}/metacall")

# MetaCall include path
find_path(METACALL_INCLUDE_DIR
	NAMES metacall/metacall.h
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall include directory"
)

# MetaCall library path
find_library(METACALL_LIBRARY
	NAMES metacall
	PATHS ${METACALL_ROOT_DIR}
	DOC "MetaCall library"
)

# Find package
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(METACALL DEFAULT_MSG METACALL_INCLUDE_DIR METACALL_LIBRARY)

if(METACALL_FOUND)
	set(METACALL_LIBRARIES ${METACALL_LIBRARY})
	set(METACALL_INCLUDE_DIRS ${METACALL_INCLUDE_DIR})
	set(METACALL_DEFINITIONS)
endif()

# Ignore local variables
mark_as_advanced(METACALL_ROOT_DIR METACALL_INCLUDE_DIR METACALL_LIBRARY)
