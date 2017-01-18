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
execute_process(COMMAND ${DOTNET_COMMAND} -v
  RESULT_VARIABLE DOTNET_COMMAND_RESULT
)

# Set found variable
if(DOTNET_COMMAND_RESULT EQUAL 0)
	set(DOTNET_FOUND TRUE)
else()
	set(DOTNET_FOUND FALSE)
endif()
