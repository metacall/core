#
#	CMake Find NPM by Parra Studios
#	CMake script to find NodeJS Package Manager.
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

# Find NPM executable and paths
#
# NPM_FOUND - True if npm was found
# NPM_GLOBAL_DIR - The global node_modules directory
# NPM_EXECUTABLE - The path to the npm executable
# NPM_VERSION - The version number

set(NPM_ROOT /usr/bin CACHE STRING "NPM directory")

find_program(NPM_EXECUTABLE
	NAMES npm
	HINTS /usr $ENV{NPM_ROOT}/npm ${NPM_ROOT}/npm
)

if(NOT NPM_EXECUTABLE)
	if(NPM_FIND_REQUIRED)
		message(FATAL_ERROR "NPM was not found")
	endif()

	return()
endif()

# On Windows set NPM to the cmd version
if(WIN32 AND EXISTS "${NPM_EXECUTABLE}.cmd")
	set(NPM_EXECUTABLE "${NPM_EXECUTABLE}.cmd")
endif()

# Get NPM version
execute_process(COMMAND ${NPM_EXECUTABLE} -v
	OUTPUT_VARIABLE NPM_VERSION
	ERROR_VARIABLE NPM_VERSION_ERROR
	RESULT_VARIABLE NPM_VERSION_CODE
)

if(NPM_VERSION_CODE)
	if(NPM_FIND_REQUIRED)
		message(FATAL_ERROR "${NPM_EXECUTABLE} -v failed:\n${NPM_VERSION_CODE}")
	endif()
endif()

if(NPM_VERSION)
	string(STRIP ${NPM_VERSION} NPM_VERSION)
endif()

# Get global node_modules location
execute_process(COMMAND ${NPM_EXECUTABLE} root -g
	OUTPUT_VARIABLE NPM_GLOBAL_DIR
	ERROR_VARIABLE NPM_GLOBAL_DIR_ERROR
	RESULT_VARIABLE NPM_GLOBAL_DIR_CODE
)

if(NPM_GLOBAL_DIR)
	string(STRIP ${NPM_GLOBAL_DIR} NPM_GLOBAL_DIR)
endif()

if(NPM_GLOBAL_DIR_CODE)
	if(NPM_FIND_REQUIRED)
		message(FATAL_ERROR "${NPM_EXECUTABLE} root -g failed:\n${NPM_GLOBAL_DIR_ERROR}")
	endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(NPM
	FOUND_VAR NPM_FOUND
	REQUIRED_VARS NPM_EXECUTABLE
	VERSION_VAR NPM_VERSION
)

mark_as_advanced(NPM_FOUND NPM_GLOBAL_DIR NPM_EXECUTABLE NPM_VERSION)
