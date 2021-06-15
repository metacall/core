#
#	WebAssembly project generator by Parra Studios
#	Generates a WebAssembly project embedded into CMake.
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

if(WASMPROJECT_FOUND)
	return()
endif()

set(WASMPROJECT_FOUND YES)

#
# Generic script project generator
#

include(ScriptProject)

# Define current Wasm project configuration path
get_filename_component(WASM_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# Wasm sub-project util function
#

function(wasm_project target version)

	# TODO

	# Configuration
	set(PACKAGE_NAME		${target})
	set(PACKAGE_VERSION		${version})
	set(PACKAGE_OUTPUT		"${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	# Create project file
    script_project(${target} wasm ${WASM_PROJECT_CONFIG_PATH}/WasmProject.cmake.in)

endfunction()
