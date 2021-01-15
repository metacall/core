
#
#	CMake Generator Expressions Utility by Parra Studios
#	Expands generator expressions during configuration phase.
#
#	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

# Usage:
# 	cmake_expand_generator_expressions(EXPANSION
# 		$<$<BOOL:${OPTION_BUILD_LOADERS_RB}>:rb_loader>
# 		$<$<BOOL:${OPTION_BUILD_LOADERS_PY}>:py_loader>
# 	)
#
# Result will be stored in EXPANSION variable:
# 	message(STATUS "${EXPANSION}")
#
# Note:
# 	This macro is very slow when run on the first time,
# 	but at least is something meanwhile this PR is not implemented:
# 	https://gitlab.kitware.com/cmake/cmake/-/issues/19467

macro(cmake_expand_generator_expressions output)
	# Convert the list of parameters into a list of arguments
	set(CMAKE_GENERATOR_EXPRESSION_LIST "${ARGN}")
	set(GENERATOR_EXPRESSION_TEMP_PATH "${CMAKE_CURRENT_BINARY_DIR}/cmake_expand_generator_expressions")
	string(REGEX REPLACE ";" " " CMAKE_GENERATOR_EXPRESSION_LIST "${CMAKE_GENERATOR_EXPRESSION_LIST}")
	string(REGEX REPLACE "\n" " " CMAKE_GENERATOR_EXPRESSION_LIST "${CMAKE_GENERATOR_EXPRESSION_LIST}")

	# Generate a CMake script for executing it (needs the build phase to evaluate the generator expressions)
	set(contents
		"file(GENERATE"
		"	OUTPUT \"${GENERATOR_EXPRESSION_TEMP_PATH}/output.tmp\""
		"	CONTENT \"${CMAKE_GENERATOR_EXPRESSION_LIST}\""
		")"
	)
	file(WRITE
		"${GENERATOR_EXPRESSION_TEMP_PATH}/CMakeLists.txt"
		${contents}
	)
	execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev ${GENERATOR_EXPRESSION_TEMP_PATH}
		WORKING_DIRECTORY ${GENERATOR_EXPRESSION_TEMP_PATH}
		OUTPUT_VARIABLE discard
	)
	file(READ
		"${GENERATOR_EXPRESSION_TEMP_PATH}/output.tmp"
		GENERATED_OUTPUT
	)

	# This is to avoid possible side effects, but I decided to remove it for caching purposes
	# file(REMOVE_RECURSE "${GENERATOR_EXPRESSION_TEMP_PATH}")

	# Clean the output and generate a list
	string(STRIP "${GENERATED_OUTPUT}" GENERATED_OUTPUT)
	string(REGEX REPLACE "\n" "" GENERATED_OUTPUT "${GENERATED_OUTPUT}")
	string(REGEX REPLACE " " ";" GENERATED_OUTPUT "${GENERATED_OUTPUT}")
	set(${output} "${GENERATED_OUTPUT}")
endmacro()
