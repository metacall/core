#
# CMake Find Dot NET Engine by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
