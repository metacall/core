#
#	Compiler checker for stack smashing flags by Parra Studios
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Tests if a defined stack smashing security flag is available.
#

include(CheckCXXSourceCompiles)

# Define current stack smashing program path
get_filename_component(stack_smashing_program_path ${CMAKE_CURRENT_LIST_FILE} PATH)

# Read source file
file(READ ${stack_smashing_program_path}/CheckCXXCompilerFlagStackSmashing.cpp stack_smashing_program)

macro(check_cxx_compiler_flag_stack_smashing _flag _result)
	# Store current definitions
	set(required_definitions "${CMAKE_REQUIRED_DEFINITIONS}")

	# Use the defined flag
	set(CMAKE_REQUIRED_DEFINITIONS "${_flag}")

	# Compile stack smashing program
	check_cxx_source_compiles("${stack_smashing_program}" ${_result})

	# Restore previous required definitions
	set(CMAKE_REQUIRED_DEFINITIONS ${required_definitions})
endmacro()
