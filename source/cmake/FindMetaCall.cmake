#
#	CMake Find MetaCall library by Parra Studios
#	CMake script to find and include MetaCall library for development.
#
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
