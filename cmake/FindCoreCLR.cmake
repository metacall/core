#
#	CMake Find CoreCLR NET Engine by Parra Studios
#	CMake script to find CoreCLR NET Engine.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
