#
#	CMake Find Zig by Parra Studios
#	CMake script to find Zig compiler and tools.
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

# Find Zig executables and paths
#
# Zig_FOUND - True if Zig was found
# Zig_COMPILER_EXECUTABLE - Zig compiler executable path

# Options
#
# Zig_CMAKE_DEBUG - Print the debug information and all constants values

option(Zig_CMAKE_DEBUG "Show full output of the Zig related commands for debugging." OFF)

find_program(Zig_COMPILER_EXECUTABLE zig
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Zig
	FOUND_VAR Zig_FOUND
)

mark_as_advanced(
	Zig_FOUND
	Zig_COMPILER_EXECUTABLE
)

if(Zig_CMAKE_DEBUG)
	message(STATUS "Zig_COMPILER_EXECUTABLE: ${Zig_COMPILER_EXECUTABLE}")
endif()
