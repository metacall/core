#
#	Compiler checker for stack smashing flags by Parra Studios
#	Tests if a defined stack smashing security flag is available.
#
#	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

include(CheckCSourceCompiles)

# Define current stack smashing program path
get_filename_component(stack_smashing_program_path ${CMAKE_CURRENT_LIST_FILE} PATH)

# Read source file
file(READ ${stack_smashing_program_path}/CheckCCompilerFlagStackSmashing.c stack_smashing_program)

macro(check_c_compiler_flag_stack_smashing _flag _result)
	# Store current definitions
	set(required_definitions "${CMAKE_REQUIRED_DEFINITIONS}")

	# Use the defined flag
	set(CMAKE_REQUIRED_DEFINITIONS "${_flag}")

	# Compile stack smashing program
	check_c_source_compiles("${stack_smashing_program}" ${_result})

	# Restore previous required definitions
	set(CMAKE_REQUIRED_DEFINITIONS ${required_definitions})
endmacro()
