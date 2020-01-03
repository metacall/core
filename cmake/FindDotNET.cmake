#
#	CMake Find Dot NET Engine by Parra Studios
#	CMake script to find DotNET Engine.
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

# Find DotNET library and include paths
#
# DOTNET_FOUND - True if DotNET was found
# DOTNET_COMMAND - Return DotNET command

# Prevent vervosity if already included
if(DOTNET_FOUND)
	set(DOTNET_FIND_QUIETLY TRUE)
endif()

# Define dotnet command
set(DOTNET_COMMAND dotnet)

# Detect dotnet command
execute_process(COMMAND ${DOTNET_COMMAND}
	RESULT_VARIABLE DOTNET_COMMAND_RESULT
	OUTPUT_QUIET
)

# Set found variable (TODO: Review 129 state in Debian)
if(DOTNET_COMMAND_RESULT EQUAL 0 OR DOTNET_COMMAND_RESULT EQUAL 129)
	set(DOTNET_FOUND TRUE)
else()
	set(DOTNET_FOUND FALSE)
endif()

# Detect dotnet variables
if(DOTNET_FOUND)
	# Detect dotnet version
	execute_process(COMMAND ${DOTNET_COMMAND} --version
		RESULT_VARIABLE DOTNET_COMMAND_RESULT
		OUTPUT_VARIABLE DOTNET_COMMAND_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if(DOTNET_COMMAND_RESULT EQUAL 0)
		set(DOTNET_VERSION "${DOTNET_COMMAND_OUTPUT}")

		# Detect dotnet migrate command
		if(DOTNET_VERSION VERSION_GREATER "1.1" OR DOTNET_VERSION VERSION_EQUAL "1.1")
			set(DOTNET_MIGRATE 1)
		else()
			set(DOTNET_MIGRATE 0)
		endif()

		mark_as_advanced(DOTNET_COMMAND DOTNET_VERSION DOTNET_MIGRATE)
	endif()
endif()
