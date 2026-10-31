#
#	Android Emulator Test by Parra Studios
#	Utility for running tests through the Android emulator when cross-compiling.
#
#	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

if(ANDROID_EMULATOR_TEST_FOUND)
	return()
endif()

set(ANDROID_EMULATOR_TEST_FOUND YES)

# Convert a path of the build directory into the path where it is located when the tests run,
# in Android the tests run in the device, where the build directory is located in ANDROID_DEVICE_DIR
function(test_runtime_path variable path)
	if(ANDROID AND CMAKE_CROSSCOMPILING_EMULATOR AND ANDROID_DEVICE_DIR)
		file(RELATIVE_PATH relative_path "${CMAKE_BINARY_DIR}" "${path}")

		# Only paths inside of the build directory are located in the device
		if(NOT relative_path MATCHES "^\\.\\.")
			if(relative_path STREQUAL "")
				set(path "${ANDROID_DEVICE_DIR}/build")
			else()
				set(path "${ANDROID_DEVICE_DIR}/build/${relative_path}")
			endif()
		endif()
	endif()

	set(${variable} "${path}" PARENT_SCOPE)
endfunction()

if(NOT (ANDROID AND CMAKE_CROSSCOMPILING_EMULATOR))
	return()
endif()

# Tests use COMMAND $<TARGET_FILE:target> but CMake only prepends the
# CMAKE_CROSSCOMPILING_EMULATOR when COMMAND is a target name, so replace it.
# The original add_test is accessible through _add_test, this file must be
# included only once, otherwise _add_test would point to this function
function(add_test)
	set(args ${ARGV})
	list(FIND args COMMAND command_index)

	if(NOT command_index EQUAL -1)
		math(EXPR executable_index "${command_index} + 1")
		list(GET args ${executable_index} executable)

		# CMAKE_MATCH_1 must be read in a separated if, it is expanded before MATCHES is evaluated
		if(executable MATCHES "^\\$<TARGET_FILE:([^>]+)>$")
			set(executable_target "${CMAKE_MATCH_1}")

			if(TARGET "${executable_target}")
				list(REMOVE_AT args ${executable_index})
				list(INSERT args ${executable_index} "${executable_target}")
			endif()
		endif()
	endif()

	_add_test(${args})
endfunction()
